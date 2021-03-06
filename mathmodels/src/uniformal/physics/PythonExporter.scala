package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._
import utils._
import objects._
import scala.util.matching.Regex

 
import info.mathhub.lf.MitM.Foundation._
import info.mathhub.lf.MitM.Foundation.Tensors._
import info.mathhub.lf.MitM.Foundation.RealLiterals._
import info.mathhub.lf.MitM.Foundation.NatLiterals._

import uniformal.physics.Quantities._
import uniformal.physics.Geometries._
import uniformal.physics.Booleans._
import scala.io.Source

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
    for (c <- s.toCharArray()){
      if (c.toInt > 127){
        mpdtool.supportedUnicodeList.indexWhere(_.contains(c)) match {
          case -1 => ;
          case i => out = out.replaceAll(c.toString, mpdtool.unicodeToLatexList(i)._2) 
          
        }
      } 
    }
    if (s.headOption.getOrElse(return s).isDigit)
      out = "_"+out
    out
  }
  
  def makePythonBooleanExpression(b: BStructure): String = {
    b match {
      case BAnd(x, y) => s"(${makePythonBooleanExpression(x)} and ${makePythonBooleanExpression(y)})"
      case BOr(x, y) => s"(${makePythonBooleanExpression(x)} or ${makePythonBooleanExpression(y)})"
      case BEqual(x, y) => s"(${makePythonNumericalExpression(x, "state")._1} == ${makePythonNumericalExpression(y, "state")._1})"
      case BGreaterThanOrEqual(x, y) => s"(${makePythonNumericalExpression(x, "state")._1} >= ${makePythonNumericalExpression(y, "state")._1})"
      case BLessThanOrEqual(x, y) => s"(${makePythonNumericalExpression(x, "state")._1} <= ${makePythonNumericalExpression(y, "state")._1})"
      case t => 
        throw LocalError("Match error when creating python boolean expression string:" + t)
    }
  }
  
  def makePyRString(rComponent: RComponent): String = rComponent match {
    case RTimes(x, y) => s"(${makePyRString(x)} * ${makePyRString(y)})"
    case RTimes10ToThePower(x, y) => s"(${makePyRString(x)} * (10.0 ** ${makePyRString(y)}))"
    case RNeg(x) => s"(-${makePyRString(x)})"
    case QTensorRealString(x) => x
    case QSymbol(name, _, _) => ensureIdentifierString(name)
    case _ => throw LocalError("Match error when creating python numerical expression string for tensor component:" + rComponent.toString)
  }
  
  def makePythonNumericalExpression(qElement: QStructure, state: String): (String, List[QSymbol])= {
    var params: List[QSymbol] = Nil
    def f(q: QStructure):String = q match {
      case QMul(x, y) => s"(${f(x)} * ${f(y)})"
      case QDiv(x, y) => s"(${f(x)} / ${f(y)})"
      case QAdd(x, y) => s"(${f(x)} + ${f(y)})"
      case QSubtract(x, y) => s"(${f(x)} - ${f(y)})"
      case QNeg(x) => s"(- ${f(x)})"
      case QExp(x) => s"numpy.exp(${f(x)})"
      case QLog(x) => s"numpy.log(${f(x)})"
      case QGradient(x) => s"gradient(${f(x)}, self.ambient_space_grid)"   
      case QDivergence(x) => s"divergence(${f(x)}, self.ambient_space_grid)" 
      case QTensorVal(l, lr) => s"${makePyTensor(l, lr.map(makePyRString(_)))}"
      case QAbs(x) => s"numpy.abs(${f(x)})"
      case QSymbol(x, _, false) => {
        params ::= q.asInstanceOf[QSymbol]
        state + "['" + ensureIdentifierString(x) + "']"
      }
      case QSymbol(x, _, true) => ensureIdentifierString(x)
      case QGetTensorComponent(t, i) => s"${f(t)}[${f(i)}]"
      case t => 
        throw LocalError("Match error when creating python numerical expression string:" + t)
    }
    (f(qElement), params.distinct)
  }
  
  private def makePyTensor(tensorRankDims: List[Int], tensorElements: List[String]): String = {
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
    
  private def makeExpressionPyLambda(state: String, value: Term, args: List[(Option[LocalName], Term)]): String = {
    makeExpressionPyLambda(state, MakeQuantityStructureFromTerm(value, args))
  }
  
  private def makeExpressionPyLambda(state: String, expr: QStructure): String =
        s"lambda $state: ${makePythonNumericalExpression(expr, state)._1}"

  
  private def makeFunctionalGraphPy(mpd: MPD): String = {
    mpd.step_graph.map(x => {
      (s"('${x.quantityDecl.name.toString}', '${x.law.name.toString}')")
    }).mkString("[", ", ", "]")
  }
  
  private def makeGraphPy(mpd: MPD): String = {
    mpd.graph.map(x => {
      (s"('${x._2.name.toString}', '${x._1.name.toString}')")
    }).mkString("[", ", ", "]")
  }
  
  private def getStringListPy(list :List[String]): String =
    list.mkString("['", "' ,'", "']")
  
  private def getObjectListPy(list: List[String]): String = 
    list.mkString("[", " ,", "]")
    
  private def makeConstQuantityExpression(value: QStructure): String = {
    makePythonNumericalExpression(value, "")._1
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
          "is_constant" -> {if (q.isConstant) "True" else "False"},
          "is_sequence" -> {if (q.isDiscreteSequence) " True" else "False"},
          "tensor_shape" -> s"[${q.tensRank.mkString(",")}]"
      )
      if (q.df != None)
        (parameters:+("initial_value" -> makeConstQuantityExpression(q.df.get))).toMap
      else  
        parameters.toMap
    })
  }
    
  private def lawsPyAttributes(mpd: MPD) = 
    mpd.laws.map(l => {
      val (lhsStr, lhsParams) = makePythonNumericalExpression(l.formula.lhs, "state")
      val (rhsStr, rhsParams) = makePythonNumericalExpression(l.formula.rhs, "state")
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
          val (stpStr, params) = makePythonNumericalExpression(stp.toRule(None).get.solution, "state")
          s"lambda state: (${stpStr})"
        }
      ).mkString("[", ",", "]")
      val lawQuantityNamePairs = bstp.steps.map(stp => 
        s"('${stp.law.name.toString}', '${stp.quantityDecl.name.toString}')"
      ).mkString("[", ",", "]")
      val stpExpression = bstp.toRule(None).get.solution
      val (stpStr, params) = makePythonNumericalExpression(stpExpression, "state")
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
    
  private def geometriesPyAttributes(mpd: MPD) = 
    mpd.geometryDecls.map(geom => {
      var hasConstruction = true
      var descriptionString: String = ""
      var geomLambdaBody = "True"
      var geomLambdaArgs: Option[String] = None
      geom.defin match {
        case Some(Geometries.GConstructionFromPredicate(pred, args)) => {
          geomLambdaBody = makePythonBooleanExpression(pred)
          assert(args.size == 1)
          geomLambdaArgs = Some(ensureIdentifierString(args(0)._1.toString))
        }
        case Some(Geometries.GConstructionFromDescription(s)) => {
          hasConstruction = false
          descriptionString = s
        }
        case None => throw new GeneralError("Geometry lacks a definition or a description: " + geom.path.toString)
      }
      val predicateLambdaStr = {
        if (hasConstruction)
          (s"lambda " + 
           {if (geomLambdaArgs != None) 
              s"${geomLambdaArgs.get}" 
            else 
              ""} + 
              s":(${geomLambdaBody})")
        else
          "None"
      }
              
      val parent = geom.path.toPath
      val name = geom.path.last
      Map(
          "name" -> s"'${name.toString}'",
          "parent" -> s"'${parent.toString}'",
          "has_construction" -> {if (hasConstruction) s"True ${1}" + s"df" else "False"},
          "description_string" -> s"'${descriptionString}'",
          "geometry_mask_predicate" -> predicateLambdaStr,
          "ambient_space_grid" -> s"self.ambient_space_grid")
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
${pyIndent(2)}self.functional_graph = ${makeFunctionalGraphPy(mpd)}
${pyIndent(2)}self.graph = ${makeGraphPy(mpd)}

${pyIndent(1)}def init_quantity_decls(self):
${pyIndent(2)}${quantityDeclsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.quantity_decls[${ensureIdentifierString(mp("name"))}]", "QuantityDecl", mp, 2)}.mkString("\n\n"+pyIndent(2))
}
${pyIndent(2)}pass

${pyIndent(1)}def init_laws(self):
${pyIndent(2)}${lawsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.laws[${ensureIdentifierString(mp("name"))}]", "Law", mp, 2)}.mkString("\n\n"+pyIndent(2))
}
${pyIndent(2)}pass

${pyIndent(1)}def init_geometries(self):
${pyIndent(2)}${geometriesPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.geometries[${ensureIdentifierString(mp("name"))}]", "Geometry", mp, 2)}.mkString("\n\n"+pyIndent(2))
}
${pyIndent(2)}pass

${pyIndent(1)}def init_computation_steps(self):
${pyIndent(2)}${computationStepsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.computation_steps[${ensureIdentifierString(mp("name"))}]", "ComputationStep", mp, 2)}.mkString("\n\n"+pyIndent(2))
}
${pyIndent(2)}pass

"""

    Some(py)
  }
  
  def exportTheory(thy: Theory, bf: BuildTask) {
    println(thy.name)
    val pyOpt = thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Foundation/Units?ModelBase" => {
         constructPyFromMPD(mpdtool.toMPD(thy))                
       }
       case _ => return
    }
        
    pyOpt.foreach(py => {
      println(py)
      println(bf.outFile.name)
      val header = "#!/usr/bin/env python\n\n# "+thy.path.toString()+"\n\n"
      utils.File.write(bf.outFile, header+py)
    })
  }

  def exportNamespace(dpath: DPath, bd: BuildTask, namespaces: List[BuildTask], modules: List[BuildTask]) {}
  
  def exportDocument(doc: Document, bf: BuildTask) {}
  
  def exportView(view: View, bf: BuildTask) {} 
}
