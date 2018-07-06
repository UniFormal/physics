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
import Units.QuantityBase._
import Units.LawBase._
import Units.GeometryBase._
import Units.TacticBase._
import Units.BoundaryConditionBase._
import info.mathhub.lf.MitM.Foundation.Tensors._


/**
 * run via build ARCHIVE mpd-python
 * e.g: build MitM/Foundation mpd-python
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
  
  private class QuantityPyExpression(state: String, quantityValue: Term, mustBeStateless: Boolean = false) {
    var parameters: List[String] = Nil
    
    val expression: String = makeQuantityPyExpression(quantityValue)
    
    private def makeQuantityPyExpression(q : Term): String = q match {
      case QuantityNeg(d, g, l, t, v) => 
        s"(- ${makeQuantityPyExpression(v)})" 
     
      case QuantityMul(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
      case QuantityLScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
      case QuantityRScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} * ${makeQuantityPyExpression(v2)})" 
      
      case QuantityDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
      case QuantityLScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
      case QuantityRScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} / ${makeQuantityPyExpression(v2)})" 
       
      case QuantityAdd(d, l, t, g1, g2, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} + ${makeQuantityPyExpression(v2)})" 
      
      case QuantitySubtract(d, g1, g2, l, t, v1, v2) => 
        s"(${makeQuantityPyExpression(v1)} - ${makeQuantityPyExpression(v2)})" 
        
      case QuantityExp(g, v) =>
        s"numpy.exp(${makeQuantityPyExpression(v)})"
        
      case QuantityLog(g, v) =>
        s"(numpy.log(${makeQuantityPyExpression(v)})"
        
      case QuantityGradient(d, g, v) =>
        s"(gradient(${makeQuantityPyExpression(v)}, self.space))" 
        
      case QuantityDivergence(d, g, n, v) =>
        s"(divergence(${makeQuantityPyExpression(v)}, self.space))" 
        
      case MakeQuantity(l, g, d, t, lr) =>
        s"(${makePyTensor(l, lr)})"
      case MakeQuantityOnGeometry(l, g, d, t, lr) =>
        s"(${makePyTensor(l, lr)})"
        
      case OMS(path) => {
        if (mustBeStateless)
          throw GeneralError("The following quantity must be stateless: " + quantityValue.toString)
        parameters ::= path.name.toString
        state + "['" + ensureIdentifierString(path.name.toString) + "']"
      }
      
      case OMV(n) => println(n.toPath) 
        n.toPath
      
     // case ApplyGeneral(f, x) => println(q)
     //   val OMS(path) = f
     //   parameters ::= path.name.toString
     //   state + "['" + ensureIdentifierString(path.name.toString) + "']" 
      
      case OMA(OMID(path), _) =>
        throw LocalError("Undefined operation: " + q.toString())
      case t => throw LocalError("Match Error:" + t.toStr(true))
    }
    
  }
  
  private def makePyTensor(tensorRankList: Term, tensorElementsList: Term): String = {
    val tensorRankDims = makeListFromListTerm(tensorRankList).map(s => s.toInt)
    val tensorElements = makeListFromListTerm(tensorElementsList)
    val numberOfElementsRequired = tensorRankDims.foldRight(1){(acc, i)=>(acc *(i)) }
    if (numberOfElementsRequired != tensorElements.size)
       throw new GeneralError("Tensor rank incompatable with number of elements: " + numberOfElementsRequired.toString)
    def form_tensor_recursive(l: List[String], dims: List[Int]) : String = {
      dims match {
        case h::t => getObjectListPy(l.grouped(dims.foldRight(1){(acc,i)=>(acc *(i))}/h).toList.map(s=>form_tensor_recursive(s, t)))
        case Nil => if (l.size == 1) l(0) else getObjectListPy(l)
      }
    }
    s"numpy.array(${form_tensor_recursive(tensorElements, tensorRankDims)})"
  }
  
  private def makeListFromListTerm(l: Term): List[String] = {
    def get_list_recursive(t: Term, tail: List[String]) : List[String] = {
      t match {
        case tcons(a, x, b) => {
          get_list_recursive(b, (x.toString)::tail)
        }
        case rnil(p) => return tail
        case nnil(p) => return tail
      }
    }
    get_list_recursive(l, Nil)
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
  
  private def getObjectListPy(list: List[String]): String = 
    list.mkString("[", " ,", "]")
    
  private def makeConstQuantityExpression(value: Term): String = {
    val expr = new QuantityPyExpression("", value, true)
    expr.expression
  }
    
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
        (parameters:+("initial_value" -> makeConstQuantityExpression(q.df.get.value))).toMap
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
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Foundation/Units?ModelBase" => {
         println(mpdtool.toMPD(thy).toString())
         constructPyFromMPD(mpdtool.toMPD(thy))
                
       }
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
