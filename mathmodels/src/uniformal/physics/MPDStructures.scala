package uniformal.physics

import info.kwarc.mmt.api._
import objects._
import utils._

import info.mathhub.lf.MitM.Foundation._

import Units.Units._
import Units.Dimensions._
import Units.QuantityBase._
import Units.GeometryBase._
import Units.BoundaryConditionBase._
import Units.TacticBase._

import uniformal.physics.Quantities._
import uniformal.physics.Geometries._

case class Formula(lhs: QStructure, rhs: QStructure, args: List[(Option[LocalName], Term)])

abstract class MPDComponent

case class GeometryDef(parent: MPath, name: LocalName, domain: GStructure) extends MPDComponent

case class GeometryDecl(parent: MPath, name: LocalName, defin: Option[GStructure]) extends MPDComponent {
  def path = parent ? name
}


case class StepDecl(parent: MPath, name: LocalName, quantityLawPairPaths: List[(GlobalName, GlobalName)]) extends MPDComponent {
  def path = parent ? name // '?' forms global name
}

trait MPDNode

case class QuantitySequenceDecl(quantityParent: MPath, quantityName: LocalName) extends MPDComponent

case class QuantityDecl(parent: MPath, name: LocalName, l: Term, geom: Option[GStructure] , dim: Term, tensRank: List[Int], df: Option[QStructure], isDiscreteSequence: Boolean, isField: Boolean, isConstant: Boolean) extends MPDComponent with MPDNode {
  def path = parent ? name // '?' forms global name
  def toQSymbol = QSymbol(name.toString, Some(path))
}

// A rule is an equation solved for a variable
case class Rule(solved: QSymbol, solution: QStructure, ruleNumber: Option[Int])

// A law is a named container of all rules of an equation/formula
case class Law(parent: MPath, name: LocalName, formula: Formula, additionalRules: List[Rule], isComputational: Boolean = true) extends MPDComponent with MPDNode{
  def path = parent ? name

  lazy val generatedRules: List[Rule] = {  
    def generateRules(lhs: QStructure, rhs: QStructure): List[(QStructure, QStructure)] ={
      lhs match {
        case QAdd(x, y) => generateRules(x, QSubtract(rhs, y)) ++ generateRules(y, QSubtract(rhs, x))
        case QSubtract(x, y) => generateRules(x, QAdd(rhs, y)) ++ generateRules(y, QSubtract(rhs, x))
        case QNeg(x) => generateRules(x, QNeg(rhs))
        case QMul(x, y) => generateRules(x, QDiv(rhs, y)) ++ generateRules(y, QDiv(rhs, x))
        case QDiv(x, y) => generateRules(x, QMul(rhs, y)) ++ generateRules(y, QMul(rhs, x))
        case QExp(x) => generateRules(x, QLog(rhs))
        case QLog(x) => generateRules(x, QExp(rhs))
        case _ => (lhs, rhs)::Nil
      }
    }
    (generateRules(formula.lhs, formula.rhs) ++ generateRules(formula.rhs, formula.lhs))
                 .filter(x => x._1.isInstanceOf[QSymbol] || x._2.isInstanceOf[QSymbol])
                 .map(x => if (x._1.isInstanceOf[QSymbol]) x else (x._2, x._1))
                 .distinct
                 .map(x => Rule(x._1.asInstanceOf[QSymbol], x._2, None))             
  }
  
  lazy val rules = (additionalRules ++ generatedRules)
  
  lazy val usedQuantities: List[QSymbol] = (formula.lhs.symbols ++ formula.rhs.symbols).distinct
  
  def uses(quantityGlobalName: GlobalName) = usedQuantities.map(_.path) contains quantityGlobalName  
  
  def isSolvableFor(quantityGlobalName: GlobalName) = allSolvableQuantities.map(_.path) contains quantityGlobalName
  
  def getSolution(q: GlobalName): Option[QStructure] = rules.find(_.solved.path == q).map(_.solution)
  
  // a law is functional for a quantity q if it can be expressed in the form q = l(a, b, c, ..) a, b, c, .. != q 
  def isFunctional(q: GlobalName) = getFunctionalRule(q) != None

  def getFunctionalRule(q: GlobalName): Option[Rule] = rules.find(x => (x.solved.path == q && 
                                               !x.solution.contains(QSymbol(q.name.toString, Some(q)))))
                
  lazy val allSolvableQuantities = rules.map(_.solved)
  
  def solvableQuantities(qs: List[QuantityDecl]) = qs.filter(rules.map(_.solved.path) contains _.path)
}

trait GeneralComputationStep

case class Step(law: Law, quantityDecl: QuantityDecl){
  // a step is function if its law is (for its quantity)
  def isFunctional = law.isFunctional(quantityDecl.path)
  
  def toRule(ruleNumber: Option[Int]): Option[Rule] = BigStep(List(this), None).toRule(ruleNumber)
}

case class BigStep(steps: List[Step], path: Option[GlobalName]){
  def end = steps.last.quantityDecl
  
  def noLawUsedTwice: Boolean = (for {x <- steps; y <- steps if x != y} yield x.law.path != y.law.path).foldRight(true){(x,y) => x && y}
  
  def noQuantityComputedTwice: Boolean = (for {x <- steps; y <- steps if x != y} yield x.quantityDecl.path != y.quantityDecl.path).foldRight(true){(x,y) => x && y}
  
  def cyclic = steps.head.law.isFunctional(steps.last.quantityDecl.path)
  
  private[this] def subcycleFree = {
    def f(l: List[Step]): Boolean = l match {
      case h::Nil => true
      case h::t => t.foldRight(true) {(x,y) => x.quantityDecl != h.quantityDecl && y} && f(t)
      case nil => true
    }
    f(steps) 
  }
  
  def linear = subcycleFree && !cyclic
  
  def cyclicLinear = subcycleFree && cyclic
  
  // linear paths are always simple
  def simple = linear || (noLawUsedTwice && noQuantityComputedTwice)
  
  // a path is functional if every step is
  def isFunctional = steps.map(_.isFunctional).foldRight(true){(x,y) => x && y}

  def isConnected = steps.foldLeft((None: Option[QuantityDecl], true)){
    (y, x) => y._1 match {
      case Some(w) => (Some(x.quantityDecl), x.law.isFunctional(x.quantityDecl.path) && 
          x.law.getFunctionalRule(x.quantityDecl.path).get.solution.contains(w.toQSymbol) && y._2)
      case None => (Some(x.quantityDecl), x.law.isFunctional(x.quantityDecl.path))
    }
  }._2
  
  def toRule(ruleNumber: Option[Int]): Option[Rule] = {
    val q: (QSymbol , QStructure) = (steps.head.quantityDecl.toQSymbol, steps.head.law.getSolution(steps.head.quantityDecl.path).getOrElse(return None))
    val t: (QSymbol, QStructure) = steps.tail.foldLeft(q) {case (sofar: (QSymbol, QStructure), next: Step) =>
       val s: QStructure = next.law.getSolution(next.quantityDecl.path).getOrElse{return None}
       (next.quantityDecl.toQSymbol, s.substitute(sofar._1, sofar._2))
    }
    Some(Rule(t._1, t._2, ruleNumber))
  }
}

// Quantity -> Law -> (Quantity -> Law)+
case class Cycle(steps: List[Step]) {
  private[this] def rotate(remain: List[Step], over: List[Step], pred: Step => Boolean): List[Step] = remain match {
    case h::t if pred(h) => h::t ++ (over.reverse)
    case h::t => rotate(t, h::over, pred)
    case _ => throw new scala.MatchError("Match error: predicate applies to none")
  }
  
  override def equals(o: Any) = o match {
  
    case Cycle(stps) => this.steps == stps
    case _ => false
  }
  
  def breakAt(q: GlobalName): BigStep = BigStep(rotate(steps, Nil, s => s.quantityDecl.path == q), None)
 
  def breakAt(from: GlobalName, to: GlobalName): (BigStep,BigStep) = {
    def splitAt(stepsleft: List[Step], q: GlobalName, r: List[Step]) : (List[Step], List[Step]) = stepsleft match {
      case h::t if h.quantityDecl.path == q => (h::t, r)
      case h::t => splitAt(t, q, h::r)
      case Nil => (stepsleft, r)
    }
    breakAt(from) match {
      case BigStep(stps, p) => {val splt = splitAt(stps, to, Nil); (BigStep(splt._1, p), BigStep(splt._2, p))}
      case _ => throw new scala.MatchError("Match error!")
    }
  }
  
  // reverse must preserve position of first quantity for proper distinct cycle filtering
  def reverse: Cycle = {
    if (steps == Nil)
      return this 
    Cycle(steps.reverse)
  }
  
  private def normalizeRot = {
    val highestQuantityHash = steps.foldLeft(scala.Int.MinValue) {(x, y) => if (x > y.quantityDecl.hashCode) x else y.quantityDecl.hashCode}
    Cycle(rotate(steps, Nil, x => x.quantityDecl.hashCode == highestQuantityHash))
  }
  
  def normalize = {
    val c1 = this.normalizeRot
    /* I realized cycles are different if they're reversed */ 
    //val c2 = this.reverse.normalizeRot
    //if (c1.hashCode() > c2.hashCode()) c1 else c2
    c1
  }
  
  def toBigStep = BigStep(steps, None)
}

// MPDs
case class MPD(parent: DPath, name: LocalName, quantityDecls: List[QuantityDecl], 
                                               laws: List[Law], 
                                               computationSteps: List[BigStep],
                                               geometryDecls: List[GeometryDecl]) {
  def path = parent ? name
  
  def quantitiesInLaw(l: Law) = quantityDecls.map(_.path) intersect l.usedQuantities
  
  def lawsUsingQuantity(p: GlobalName) = laws.filter(_.uses(p))
  
  /** all cycles of this MPD */
  lazy val cycles: List[Cycle] = {
    var cycleAgg: List[Cycle] = Nil
    def traverse(hist: List[Step]): Unit = {
      val start = hist.head
      for {e <- step_graph if !e.quantityDecl.isConstant && e.law.path != hist.head.law.path && e.law.uses(hist.head.quantityDecl.path)}{
        if (e.law.isSolvableFor(e.quantityDecl.path)){
          if (! (hist contains e)){ // history doesn't contain entire edge: consider adding to history and recursing
            if (!(hist.map(_.quantityDecl) contains e.quantityDecl)){ // but if q is there, edge is partially in; kill it
              traverse(e::hist) 
            }
            
          }else if (hist.length > 2 && e == hist.last){
            cycleAgg ::= Cycle(hist).reverse.normalize
          }
        }
      }
    }
    
    for {e <- step_graph}{
      traverse(e::Nil)
    }
    
    cycleAgg.distinct
  }
  
  lazy val step_graph: List[Step] =
    laws.foldLeft(Nil: List[Step]){
     (agg, l) => agg ++ l.solvableQuantities(quantityDecls).map(q => Step(l, q))
    }
    
  lazy val graph: List[(GlobalName, GlobalName)] = 
    laws.foldLeft(Nil: List[(GlobalName, GlobalName)]){
     (agg, l) => agg ++ l.usedQuantities.map(q => (l.path, q.path.getOrElse(throw new GeneralError("Symbol has no global path and might therefore by a parameter"))))
    }
  
  def prettyListCycles = cycles.map(_.steps.map(x=>(x.law.name.toString(), x.quantityDecl.name.toString())))
  
    //List(cycles2(0), cycles2(0).reverse).map(_.steps.map(x=>(x.law.name, x.quantityDecl.name)))
  
}
