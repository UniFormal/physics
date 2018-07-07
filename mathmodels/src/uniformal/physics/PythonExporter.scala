package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._
import utils._
import objects._
 
import info.mathhub.lf.MitM.Foundation._
import info.mathhub.lf.MitM.Foundation.Tensors._


class PythonExporter extends Exporter {
  override val outExt = "py"
  
  val key = "mpd-python"
  
  private lazy val mpdtool = {
    val m = new MPDTool(controller)
    m
  }
  
  private def pyIndent(n: Int): String = "\t"*n
  
  private def ensureIdentifierString(s :String): String = {
    var out: String = s
    if (s.headOption.getOrElse(return s).isDigit)
      out = "_"+out
    out
  }
  
  def makePythonExpression(qElement: QElement) = qElement match {
    case QMul(x, y) => s"(${x} * ${y})"
    case QDiv(x, y) => s"(${x} / ${y})"
    case QAdd(x, y) => s"(${x} + ${y})"
    case QSubtract(x, y) => s"(${x} - ${y})"
    case QNeg(x) => s"(- ${x})"
    case QExp(x) => s"numpy.exp(${x})"
    case QLog(x) => s"(numpy.log(${x})"
    case QGradient(x) => s"(gradient(${x}, self.space))"   
    case QDivergence(x) => s"(divergence(${x}, self.space))" 
    case QTensorVal(l, lr) => s"(${makePyTensor(l, lr)})"
    case QSymbol(x, state) => state + "['" + ensureIdentifierString(x) + "']"
    case t => throw LocalError("Match Error:" + t)
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
  
  
  private def makeExpressionPyLambda(state: String , qexpr: QuantityStringExpression): String = 
    s"lambda $state: ${qexpr.expression}"
  
  private def makeExpressionPyLambda(state: String, value: Term): String = {
    val expr = new QuantityStringExpression(makePythonExpression, state, value)
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
    val expr = new QuantityStringExpression(makePythonExpression, "", value, true)
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
      val lawLhsExpr = new QuantityStringExpression(makePythonExpression, "state", l.formula.lhs)
      val lawRhsExpr = new QuantityStringExpression(makePythonExpression, "state", l.formula.rhs)
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
