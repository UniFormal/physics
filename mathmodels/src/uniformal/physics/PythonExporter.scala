package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._
import utils._
import objects._

import info.mathhub.lf.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QEBase._

/**
 * run via build mpd-python ARCHIVE
 */
class PythonExporter extends Exporter {
  
  override val outExt = "py"
  
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
    
    val expression = makeQuantityPyExpression(quantityValue)
    
    private def makeQuantityPyExpression(q : Term): String = q match {
      case QEMul(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
      case QEDiv(d1, d2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
      case QEAdd(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} + ${makeQuantityPyExpression(v2)})" 
      case QESubtract(d, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} - ${makeQuantityPyExpression(v2)})" 
      case OMS(path) => {
          parameters ::= path.name.toString
          state + "['" + ensureIdentifierString(path.name.toString) + "']" 
      }
      case l: OMLITTrait => l.toString
      case OMV(n) => n.toPath
      case OMA(OMID(path), _) =>
        throw LocalError("Undefined operation: " + path.toString())
      case t => throw LocalError("Match Error:" + t.toStr(true))
    }
    
  }
  
  private def makeExpressionPyLambda(state: String , qexpr: QuantityPyExpression): String = 
    s"lambda $state: ${qexpr.expression}"
  
  private def makeExpressionPyLambda(state: String, value: Term): String = {
    val expr = new QuantityPyExpression(state, value)
    makeExpressionPyLambda(state, expr)
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
          })
      )
      if (q.df != None)
        (parameters:+("compute" -> makeExpressionPyLambda("state", q.df.get.value))).toMap
      else  
        parameters.toMap
    })
  }
    
  private def lawsPyAttributes(mpd: MPD) = 
    mpd.laws.map(l => {Map(
        "name" -> s"'${l.name.toString}'",
        "parent" -> s"'${l.parent.toString}'",
        "variables" -> s"${getStringListPy(l.usedQuantities.map(_.name.toString))}") ++ 
        l.rules.map(r => (r.solved.name.toString, makeExpressionPyLambda("state", r.solution.value)))
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
${pyIndent(1)}def __init__(self, regions=1):
${pyIndent(2)}MPDBase.__init__(self, 
${pyIndent(3)}regions,
${pyIndent(3)}'${mpd.name.toString}',
${pyIndent(3)}'${mpd.parent.toString}')

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
