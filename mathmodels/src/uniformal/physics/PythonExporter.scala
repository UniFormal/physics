package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._
import utils._
import objects._

import info.kwarc.mmt.lf._

import info.mathhub.lf.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QEBase._
import Units.Field._

/**
 * run via build mpd-python ARCHIVE
 */
class PythonExporter extends Exporter {
  
  override val outExt = "py"
  
  val constants = Map("zero" -> "0", "one" -> "1", "pi" -> "numpy.pi")
  
  val key = "mpd-python"
  private lazy val mpdtool = {
    val m = new MPDTool
    initOther(m)
    m
  }
  
  private def pyIndent(n: Int): String = "\t"*n
  
  private def ensureIdentifierString(s :String): String = {
    var out: String = s
    if (s.headOption.getOrElse(return s).isDigit)
      out = "_"+out
    out
  }
  
  private class QuantityPyExpression(state: String, quantityValue: Term) {
    var parameters: List[String] = Nil
    
    val expression: String = makeQuantityPyExpression(quantityValue)
    
    private def makeQuantityPyExpression(q : Term): String = q match {
      case QEMinus(d, v) => 
        s"(- ${makeQuantityPyExpression(v)})" 
      case FieldMinus(d, v) => 
        s"(- ${makeQuantityPyExpression(v)})" 
        
      case QEMul(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
      case FieldMul(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
        
      case QEDiv(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
      case FieldDiv(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
        
      case QEAdd(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} + ${makeQuantityPyExpression(v2)})" 
      case FieldAdd(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} + ${makeQuantityPyExpression(v2)})" 
        
      case QESubtract(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} - ${makeQuantityPyExpression(v2)})" 
      case FieldSubtract(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} - ${makeQuantityPyExpression(v2)})" 
        
      case QEExp(v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} ** ${makeQuantityPyExpression(v2)})" 
      case FieldExp(v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} ** ${makeQuantityPyExpression(v2)})"
        
      case QEEExp(v1) =>
        s"numpy.exp(${makeQuantityPyExpression(v1)})"
      case FieldEExp(v1) =>
        s"numpy.exp(${makeQuantityPyExpression(v1)})"
        
      case QELog(v1, v2) =>
        s"(numpy.log(${makeQuantityPyExpression(v1)})/numpy.log(${makeQuantityPyExpression(v2)}))"
      case QEELog(v1) =>
        s"numpy.log(${makeQuantityPyExpression(v1)})"
        
      case FieldEExp(v1) =>
        s"numpy.exp(${makeQuantityPyExpression(v1)})"

      case FieldLog(v1, v2) =>
        s"(numpy.log(${makeQuantityPyExpression(v1)})/numpy.log(${makeQuantityPyExpression(v2)}))"
      case FieldELog(v1) =>
        s"numpy.log(${makeQuantityPyExpression(v1)})"
        
      case FieldDeriv(d, v) => 
        s"(derivative_on_space(${makeQuantityPyExpression(v)}, self.space))" 
        
      case OMS(path) => {
        val name = path.name.toString
        if (constants contains name)
          constants(name)
        else{
          parameters ::= path.name.toString
          state + "['" + ensureIdentifierString(path.name.toString) + "']"
        }
      }
      
      case OMV(n) => println(n.toPath)
        if (constants contains n.toPath)
          constants(n.toPath)
        else 
          n.toPath
      
      case ApplyGeneral(f, x) => println(q)
        val OMS(path) = f
        parameters ::= path.name.toString
        state + "['" + ensureIdentifierString(path.name.toString) + "']" 
      
      case OMA(OMID(path), _) =>
        throw LocalError("Undefined operation: " + q.toString())
      case t => throw LocalError("Match Error:" + t.toStr(true))
    }
    
  }
  
  private def makeExpressionPyLambda(state: String , qexpr: QuantityPyExpression): String = 
    s"lambda $state: ${qexpr.expression}"
  
  private def makeExpressionPyLambda(state: String, value: Term): String = {
    val expr = new QuantityPyExpression(state, value)
    makeExpressionPyLambda(state, expr)
  }
  
  private def makeGraphPy(mpd: MPD): String = {
    mpd.graph.map(x => {
      (s"('${x.quantityDecl.name.toString}', '${x.law.name.toString}')")
    }).mkString("[", ", ", "]")
  }
  
  private def getStringListPy(list :List[String]): String =
    list.mkString("['", "' ,'", "']")
  
  private def quantityDeclsPyAttributes(mpd: MPD) = {
    mpd.quantityDecls.map(q => {
      val parameters = List(
          "name" -> s"'${q.name.toString}'",
          "parent" -> s"'${q.parent.toString}'",
          "dimension" -> (q.dim match {
            case OMS(p) => s"'${p.name.toString}'"
            case _ => "''"
          }),
          "is_field" -> {if (q.isField) "True" else "False"} ,
          "is_constant" -> {if (q.isConstant) "True" else "False"}
      )
      if (q.df != None)
        (parameters:+("compute" -> makeExpressionPyLambda("state", q.df.get.value))).toMap
      else  
        parameters.toMap
    })
  }
    
  private def lawsPyAttributes(mpd: MPD) = 
    mpd.laws.map(l => {
      val lawLhsExpr = new QuantityPyExpression("state", l.formula.lhs)
      val lawRhsExpr = new QuantityPyExpression("state", l.formula.rhs)
      val out = Map(
          "name" -> s"'${l.name.toString}'",
          "parent" -> s"'${l.parent.toString}'",
          "used_quantities" -> s"${getStringListPy(lawRhsExpr.parameters ++ lawLhsExpr.parameters)}",
          "law_test" -> s"lambda state: ((${lawLhsExpr.expression}) - (${lawRhsExpr.expression}))")
          
      if (l.isComputational)
        out ++ Map("solvables" -> s"${getStringListPy(l.usedQuantities.map(_.name.toString))}") ++
          l.rules.map(r => (r.solved.name.toString, makeExpressionPyLambda("state", r.solution.value)))
      else
        out ++ Map("solvables" -> "[]")
    })
  
  def pyObjectAssignment(lhs: String, objname: String, attrs: Map[String, String], currentIndentLevel: Int): String = {
    s"""$lhs = $objname(
${pyIndent(currentIndentLevel+1)}${attrs.map(kp => s"${kp._1} = ${kp._2}").mkString(",\n"+pyIndent(currentIndentLevel+1))} 
${pyIndent(currentIndentLevel)})"""
  }
    
  def constructPyFromMPD(mpdOpt: Option[MPD]) : Option[String] = {
    val mpd = mpdOpt.getOrElse(return None)
    val py = s"""
from mpdbase import *

class MPD_${mpd.name.toString()}(MPDBase):
${pyIndent(1)}def __init__(self, space, integration_surfaces=[]):
${pyIndent(2)}MPDBase.__init__(self, 
${pyIndent(3)}'${mpd.name.toString}',
${pyIndent(3)}'${mpd.parent.toString}',
${pyIndent(3)}space,
${pyIndent(3)}integration_surfaces)
${pyIndent(2)}self.graph = ${makeGraphPy(mpd)}

${pyIndent(1)}def init_quantity_decls(self):
${pyIndent(2)}${quantityDeclsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.quantity_decls[${mp("name")}]", "QuantityDecl", mp, 2)}.mkString("\n\n"+pyIndent(2))
}

${pyIndent(1)}def init_laws(self):
${pyIndent(2)}${lawsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.laws[${mp("name")}]", "Law", mp, 2)}.mkString("\n\n"+pyIndent(2))
}


"""

    Some(py)
  }
  
  def exportTheory(thy: DeclaredTheory, bf: BuildTask) {
    println(thy.name)
    val pyOpt = thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Models?MPD" => constructPyFromMPD(mpdtool.toMPD(thy))
       case _ => return
    }
    
    pyOpt.foreach(py => {
      println(py)

      val header = "#!/usr/bin/env python\n\n# "+thy.path.toString()+"\n\n"
      utils.File.write(bf.outFile, header+py)
    })
  }

  def exportNamespace(dpath: DPath, bd: BuildTask, namespaces: List[BuildTask], modules: List[BuildTask]) {}
  
  def exportDocument(doc: Document, bf: BuildTask) {}
  
  def exportView(view: DeclaredView, bf: BuildTask) {}
}

/*
 * 
 */
