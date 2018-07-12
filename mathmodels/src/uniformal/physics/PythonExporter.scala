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
import info.mathhub.lf.MitM.Foundation.RealLiterals._


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
  
  def makePythonExpression(qElement: QElement, state: String): (String, List[QSymbol])= {
    var params: List[QSymbol] = Nil
    def f(q: QElement):String = q match {
      case QMul(x, y) => s"(${f(x)} * ${f(y)})"
      case QDiv(x, y) => s"(${f(x)} / ${f(y)})"
      case QAdd(x, y) => s"(${f(x)} + ${f(y)})"
      case QSubtract(x, y) => s"(${f(x)} - ${f(y)})"
      case QNeg(x) => s"(- ${f(x)})"
      case QExp(x) => s"numpy.exp(${f(x)})"
      case QLog(x) => s"numpy.log(${f(x)})"
      case QGradient(x) => s"gradient(${f(x)}, self.space)"   
      case QDivergence(x) => s"divergence(${f(x)}, self.space)" 
      case QTensorVal(l, lr) => s"${makePyTensor(l, lr)}"
      case QSymbol(x, _) => {
        params ::= q.asInstanceOf[QSymbol]
        state + "['" + ensureIdentifierString(x) + "']"
      }
      case t => throw LocalError("Match Error:" + t)
    }
    (f(qElement), params.distinct)
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
          get_list_recursive(b, (makeRealStringFromRealTerm(x))::tail)
        }
        case rnil(p) => return tail
        case nnil(p) => return tail
      }
    }
    get_list_recursive(l, Nil)
  }
  
  private def makeRealStringFromRealTerm(r:Term): String = {
    r match {
      case real_underscore_times_underscore_tenth_underscore_pow(a, b) => 
        (makeRealStringFromRealTerm(a).toDouble * scala.math.pow(10.0,
                       makeRealStringFromRealTerm(b).toDouble)).toString
      case times_underscore_real_underscore_lit(a, b) =>
        (makeRealStringFromRealTerm(a).toDouble *
         makeRealStringFromRealTerm(b).toDouble).toString
      case minus_underscore_real_underscore_lit(a) =>
        (- makeRealStringFromRealTerm(a).toDouble).toString
      case a => a.toString
    }
  }
  
  private def makeExpressionPyLambda(state: String , qexpr: QuantityExpression): String = 
    s"lambda $state: ${makeExpressionPyLambda(state, qexpr.expr)}"
  
  private def makeExpressionPyLambda(state: String, value: Term): String = {
    val expr = new QuantityExpression(value)
    makeExpressionPyLambda(state, expr)
  }
  
  private def makeExpressionPyLambda(state: String, expr: QElement): String =
        s"lambda $state: ${makePythonExpression(expr, state)._1}"

  
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
    val expr = new QuantityExpression(value, true)
    makePythonExpression(expr.expr, "")._1
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
          "is_uniform" -> {if (q.isUniform) "True" else "False"} ,
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
      val lawLhsExpr = new QuantityExpression(l.formula.lhs)
      val (lhsStr, lhsParams) = makePythonExpression(lawLhsExpr.expr, "state")
      val lawRhsExpr = new QuantityExpression(l.formula.rhs)
      val (rhsStr, rhsParams) = makePythonExpression(lawRhsExpr.expr, "state")
      val out = Map(
          "name" -> s"'${l.name.toString}'",
          "parent" -> s"'${l.parent.toString}'",
          "used_quantities" -> s"${getStringListPy(((lhsParams ++ rhsParams).map(x=>x.name)).distinct)}",
          "law_test" -> s"lambda state: ((${rhsStr}) - (${lhsStr}))")
          
      if (l.isComputational)
        out ++ Map("solvables" -> s"${getStringListPy(l.usedQuantities.map(_.name.toString))}") ++
          l.rules.map(r => (r.solved.name.toString, makeExpressionPyLambda("state", r.solution)))
      else
        out ++ Map("solvables" -> "[]")
    })
    
  private def computationStepsPyAttributes(mpd: MPD) = 
    mpd.computationSteps.map(bstp => {
      val substepsLambdas = bstp.steps.map(stp => {
          val (stpStr, params) = makePythonExpression(stp.toRule(None).get.solution, "state")
          s"lambda state: (${stpStr})"
        }
      ).mkString("[", ",", "]")
      val lawQuantityNamePairs = bstp.steps.map(stp => 
        s"('${stp.law.name.toString}', '${stp.quantityDecl.name.toString}')"
      ).mkString("[", ",", "]")
      val stpExpression = bstp.toRule(None).get.solution
      val (stpStr, params) = makePythonExpression(stpExpression, "state")
      val parent = bstp.path.get.toPath
      val name = bstp.path.get.last
      Map(
          "name" -> s"'${name.toString}'",
          "parent" -> s"'${parent.toString}'",
          "used_quantities" -> s"${getStringListPy(((params).map(x=>x.name)).distinct)}",
          "compute" -> s"lambda state: (${stpStr})",
          "number_of_substeps" -> s"${bstp.steps.size}",
          "substeps" -> substepsLambdas,
          "law_quantity_pairs" -> lawQuantityNamePairs,
          "is_cyclic" -> {if (bstp.cyclic) "True" else "False"},
          "is_connected" -> {if (bstp.isConnected) "True" else "False"})
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

${pyIndent(1)}def init_computation_steps(self):
${pyIndent(2)}${computationStepsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.computation_steps[${mp("name")}]", "ComputationStep", mp, 2)}.mkString("\n\n"+pyIndent(2))
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
