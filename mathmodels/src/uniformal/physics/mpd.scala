package uniformal.physics

import info.kwarc.mmt.api._
import objects._
import utils._

import info.mathhub.lf.MitM.Foundation._

import Units.Units._
import Units.Dimensions._
import Units.QEBase._

/*
 
case class
 
 */

case class Quantity(value: Term, tp:Term) {
  def times(q: Quantity) = {
    (tp, q.tp) match {
       case (QE(d), QE(e)) => Quantity(QEMul(d, e, value, q.value), QE(DimTimes(d,e)))
       case _ => throw new scala.MatchError("Error times")
    }
  }
  def div(q: Quantity) = {
    (tp, q.tp) match {
       case (QE(d), QE(e)) => Quantity(QEDiv(d, e, value, q.value), QE(DimDiv(d,e)))
       case _ => throw new scala.MatchError("Error")
    }
  }
  def add(q: Quantity) = {
    (tp, q.tp) match {
       case (QE(d), QE(e)) if d == e => Quantity(QEAdd(d, value, q.value), QE(d))
       case _ => throw new scala.MatchError("Error")
    }
  }
  def sub(q: Quantity) = {
    (tp, q.tp) match {
       case (QE(d), QE(e)) if d == e => Quantity(QESubtract(d, value, q.value), QE(d))
       case _ => throw new scala.MatchError("Error")
    }
  }
  
  def equal(q: Quantity): Formula = {
    (tp, q.tp) match {
       case (QE(d), QE(e)) if d == e => Formula(Logic._eq(d, value, q.value))
       case _ => throw new scala.MatchError("Error")
    }
  }
  
  private class Replacer(path: GlobalName, term: Term) extends StatelessTraverser {
    def traverse(t: Term)(implicit con : Context, state : State) = t match {
      case OMS(p) if p == path => term
      case _ => Traverser(this, t)
    }
  }
  
  def substitute(p: GlobalName, q: Quantity) = {
    val vR = (new Replacer(p, q.value)).apply(value, Context.empty)
    Quantity(vR, tp)
  }
  
  
  private class Finder(path: GlobalName) extends StatelessTraverser {
    var found = false
    def traverse(t: Term)(implicit con : Context, state : State) = t match {
      case OMS(p) if p == path => {
        found = true
        Traverser(this, t)
      }
      case _ =>Traverser(this, t)
    }
  }
  
  def contains(q: GlobalName) = {
    var vF = new Finder(q)
    vF.apply(value, Context.empty)
    vF.found
  }
  
}

case class Formula(value: Term)


abstract class MPDComponent

trait MPDNode

case class QuantityDecl(parent: MPath, name: LocalName, arguments: List[Term], dim: Term, df: Option[Quantity]) extends MPDComponent with MPDNode {
  def path = parent ? name // '?' forms global name
  def toQuantity = Quantity(OMS(path), QE(dim))
}

// A rule is an equation solved for a variable
case class Rule(solved: GlobalName, solution: Quantity)

// A law is a named container of all rules of an equation/formula
case class Law(parent: MPath, name: LocalName, formula: Formula, rules: List[Rule]) extends MPDComponent with MPDNode{
  def path = parent ? name
  
  // this method doesn't seem to recurse, so it's useless for now
  lazy val globalNames = {
    formula.value.subobjects.collect {
      case (_,OMS(p)) => p
    }.distinct.toList
  }
  
  def usedQuantities = rules.map(_.solved)
  
  def uses(quantityGlobalName: GlobalName) = usedQuantities contains quantityGlobalName  
    
  def getSolution(q: GlobalName): Option[Quantity] = rules.find(_.solved == q).map(_.solution)
  
  // a law is functional for a quantity q if it can be expressed in the form q = l(a, b, c, ..) a, b, c, .. != q 
  def isFunctional(q: GlobalName) = rules.find(_.solved == q) == None || rules.find(x => x.solved == q && x.solution.contains(q)) == None

  def solvableQuantities(qs: List[QuantityDecl]) = qs.filter(rules.map(_.solved) contains _.path)
}

case class Step(law: Law, quantityDecl: QuantityDecl){
  // a step is function if its law is (for its quantity)
  def isFunctional = law.isFunctional(quantityDecl.path)
}

case class BigStep(steps: List[Step]) {
  def end = steps.last.quantityDecl
  
  def noLawUsedTwice: Boolean = (for {x <- steps; y <- steps if x != y} yield x.law.path != y.law.path).foldRight(true){(x,y) => x && y}
  
  def noQuantityComputedTwice: Boolean = (for {x <- steps; y <- steps if x != y} yield x.quantityDecl.path != y.quantityDecl.path).foldRight(true){(x,y) => x && y}
  
  def cyclic = !steps.head.law.solvableQuantities(List(steps.last.quantityDecl)).isEmpty
  
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
  
  def toRule: Option[Rule] = {
    val q = steps.last.quantityDecl
    val t = steps.foldRight(q.toQuantity) {case (next, sofar) =>
       val s = next.law.getSolution(next.quantityDecl.path).getOrElse{return None}
       sofar.substitute(next.quantityDecl.path, s)
    }
    Some(Rule(q.path, t))
  }
}

// Constant -> Law -> (Constant -> Law)+
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
  
  def breakAt(q: GlobalName): BigStep = BigStep(rotate(steps, Nil, s => s.quantityDecl.path == q))
 
  def breakAt(from: GlobalName, to: GlobalName): (BigStep,BigStep) = {
    def splitAt(stepsleft: List[Step], q: GlobalName, r: List[Step]) : (List[Step], List[Step]) = stepsleft match {
      case h::t if h.quantityDecl.path == q => (h::t, r)
      case h::t => splitAt(t, q, h::r)
      case Nil => (stepsleft, r)
    }
    breakAt(from) match {
      case BigStep(stps) => {val splt = splitAt(stps, to, Nil); (BigStep(splt._1), BigStep(splt._2))}
      case _ => throw new scala.MatchError("Match error!")
    }
  }
  
  // reverse must preserve position of first quantity for proper distinct cycle filtering
  def reverse: Cycle = {
    if (steps == Nil)
      return this 
    val newInitialStep = Step(steps.last.law, steps.head.quantityDecl)
    val reversedStepsPair  = steps.tail.foldRight((newInitialStep::Nil, steps.map(_.law).reverse)) {
      (y, x) => x match {
        case (a:List[Step], h::b) => (Step(b.head, y.quantityDecl)::a, b)
        case _ => throw new scala.MatchError("Match error!")
      }
    }
    Cycle(reversedStepsPair._1.reverse)
  }
  
  private def normalizeRot = {
    val highestQuantityHash = steps.foldLeft(scala.Int.MinValue) {(x, y) => if (x > y.quantityDecl.hashCode) x else y.quantityDecl.hashCode}
    Cycle(rotate(steps, Nil, x => x.quantityDecl.hashCode == highestQuantityHash))
  }
  
  def normalize = {
    val c1 = this.normalizeRot
    val c2 = this.reverse.normalizeRot
    if (c1.hashCode() > c2.hashCode()) c1 else c2
  }
  
  def toBigStep = BigStep(steps)
}

// MPDs
case class MPD(parent: DPath, name: LocalName, quantityDecls: List[QuantityDecl], laws: List[Law]) {
  def path = parent ? name
  
  def quantitiesInLaw(l: Law) = quantityDecls.map(_.path) intersect l.usedQuantities
  
  def lawsUsingQuantity(p: GlobalName) = laws.filter(_.uses(p))
  
  /** all cycles of this MPD */
  lazy val cycles: List[Cycle] = {
    var cycleAgg: List[Cycle] = Nil
    
    def traverse(hist: List[Step]): Unit = {
      val start = hist.head
      for {e <- graph if e.law.path != hist.head.law.path}{
        if (e.law.uses(hist.head.quantityDecl.path)){
          if (! (hist contains e)){ // history doesn't contain entire edge: consider adding to history and recursing
            if (!(hist.map(_.quantityDecl) contains e.quantityDecl)) // but if q is there, edge is partially in; kill it
                 traverse(e::hist)
          }else if (hist.length > 2 && e == hist.last){
            cycleAgg ::= Cycle(hist).normalize
          }
        }
      }
    }
    
    for {e <- graph}{
      traverse(e::Nil)
    }
    
    cycleAgg.distinct
  }
  
  lazy val graph: List[Step] =
    laws.foldLeft(Nil: List[Step]){
     (agg, l) => agg ++ l.solvableQuantities(quantityDecls).map(q => Step(l, q))
    }
  
  
  
  def prettyListCycles = cycles.map(_.reverse.steps.map(x=>(x.law.name, x.quantityDecl.name)))
  
    //List(cycles2(0), cycles2(0).reverse).map(_.steps.map(x=>(x.law.name, x.quantityDecl.name)))
  
}

//don't need mutable
//import scala.collection.mutable.Map

// an assignment of physical quantities to every quantity declaration
case class MPDState(value: Map[QuantityDecl, Option[ImpreciseFloat]]){
  /** recompute one quantity using one law and the current values of the other quantities */
  def compute(s: Step): MPDState = {
    // result of applying s.law to current values 
    val solution = s.law.rules.find(_.solved == s.quantityDecl.path).getOrElse{throw new scala.Error("Couldn't compute")}.solution
    MPDState(value + (s.quantityDecl -> Some(EvaluateQuantity(solution, valueNameMap))))   
  }
  
  def compute(p: BigStep) : MPDState = {
    def recurse(l: List[Step], s: MPDState): MPDState = l match {
      case h::Nil => s.compute(h)
      case h::t => recurse(t, s.compute(h))
      case Nil => throw new scala.MatchError("Match error");
    }
    recurse(p.steps, this)
  }
  
  def stable(mpd: MPD, precision: Precision): Boolean = {
    mpd.laws.foldRight(true){(x, y)=>testStabilityForLaw(x, precision) && y}
  }
  
  def testStabilityForLaw(l: Law, precision: Precision) : Boolean = {
    l.rules.foldRight(true) {(x, y)=>(testStabilityForRule(x, precision) && y)}
  }
  
  def testStabilityForRule(r: Rule, precision: Precision) : Boolean = {
    val subjectQuantity = value.find(_._1.name == r.solved.name).get._1
    val diff = (EvaluateQuantity(r.solution, valueNameMap) - valueNameMap(r.solved.name).get).abs
    diff <= precision.precision(subjectQuantity).get.abs
  }
  
  def valueNameMap = value.map(j => (j._1.name, j._2))
  
  def -(rhs: MPDState): Precision = Precision(rhs.value.map(x => (x._1, Some(value(x._1).get - x._2.get ))) )
}

abstract class Tactic {
  def check: Boolean
}

case class Fixpoint(path: BigStep, initialValue: Float) extends Tactic {
  /** must be cyclic-linear */
  def check: Boolean = path.cyclicLinear
}

case class Confluence(path1: BigStep, path2: BigStep) extends Tactic {
  /** both must be linear, must end in same quantity e, e must be connected to first law of each */
  def check: Boolean = path1.linear && path2.linear && 
    path1.end == path2.end &&
    path1.steps.head.law == path2.steps.head.law
}

/** strategies are composed tactics */
case class Compose(tactics: List[Tactic]) extends Tactic {
  def check: Boolean = tactics.forall(_.check)
}

case class Precision(precision: Map[QuantityDecl, Option[ImpreciseFloat]]) {
  def isWithin(that: Precision): Boolean = that.precision.keys.foldLeft(true){(x, y)=>x && precision(y).get <= that.precision(y).get }
  def abs: Precision = Precision(precision.map(x=>(x._1, Some(x._2.get.abs))))
}

class MPDSolver(val mpd: MPD, val initialState: MPDState) {
  private var state = initialState
  def getState = state
  
  def solve(tactic: Tactic, precision: Precision) {
    applyTactic(tactic, precision)
  }
  
  def applyTactic(t: Tactic, precision: Precision) {
     t match {
       case Fixpoint(p, i) => {
         if (!t.check)
           return
         
         val fixedpointRule = p.toRule.get
         val f = utils.File("mpdout/outmpd.json")
         utils.File.write(f, RuleToJSON(fixedpointRule).toString())
         
         // assert
         val subjectQuantity = p.steps.last.quantityDecl
         if (subjectQuantity.name != fixedpointRule.solved.name)
           throw new scala.Error("Should be head")
         
         
         if (state.value(subjectQuantity).isEmpty) {
            val newS = state.value.map {x => 
               if (x._1 == subjectQuantity) 
                 (x._1, Some(ImpreciseFloat(i,0,0)))
               else x
            }
            state = MPDState(newS)
         }
         var before: MPDState = state
         do{
           // first we compute the next value
           var next: ImpreciseFloat = EvaluateQuantity(fixedpointRule.solution, state.valueNameMap)
           
           // then we update the state with the new improved value and the other values computed from chaining
           state = MPDState(state.value.map(x=> 
               if (x._1 == subjectQuantity) 
                 (x._1, Some(next))
               else x)).compute(p)
         } while ((state - before).abs.isWithin(precision))
         
       }
       
         // print fixpoint equation and current state to file
         // see utils.File, utils.JSON
       case Confluence(p1,p2) =>
         // print equation and current state to file
       case Compose(ts) =>
         var before: MPDState = null
         do {
           before = state
           ts.foreach(applyTactic(_, precision))
         }
         while ((state - before).abs.isWithin(precision))
     }
  }
}

object EvaluateQuantity {
  def apply(quantity: Quantity, vMap: Map[LocalName, Option[ImpreciseFloat]]): ImpreciseFloat = {
    quantity match {   
      case Quantity(QEMul(d1, d2, v1, v2), ft) => 
        EvaluateQuantity(Quantity(v1, ft), vMap) * EvaluateQuantity(Quantity(v2, ft), vMap) 
      case Quantity(QEDiv(d1, d2, v1, v2), ft) => 
        EvaluateQuantity(Quantity(v1, ft), vMap) / EvaluateQuantity(Quantity(v2, ft), vMap) 
      case Quantity(QEAdd(d, v1, v2), ft) => 
        EvaluateQuantity(Quantity(v1, ft), vMap) + EvaluateQuantity(Quantity(v2, ft), vMap)
      case Quantity(QESubtract(d, v1, v2), ft) => 
        EvaluateQuantity(Quantity(v1, ft), vMap) - EvaluateQuantity(Quantity(v2, ft), vMap)
      case Quantity(OMS(path), ft) if vMap(path.name).get != None => return vMap(path.name).get
      case _ => throw new scala.MatchError("Math Error")
    }
  }
}

object RuleToJSON {
  def JSONifyQuantity(q: Quantity): JSON = doQ(q.value)
  private def doQ(t: Term): JSON = t match {
    case OMA(OMID(path), List(d1,d2,v1,v2)) => 
      JSONObject(
        path.name.toString -> JSONArray(doQ(v1), doQ(v2))
      )
    case OMA(OMID(path), List(d,v1,v2)) => 
      JSONObject(
        path.name.toString -> JSONArray(doQ(v1), doQ(v2))
      )
    case OMS(path) => JSONString(path.name.toString)
  }
  
  def apply(r : Rule) = {
    JSONObject("Equal" -> JSONArray(JSONString(r.solved.name.toString), JSONifyQuantity(r.solution)))
  }
}

