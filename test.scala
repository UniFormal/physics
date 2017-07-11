abstract class Dimension{}

class Scalar extends Dimension{}

// Quantities

import Dimensions._
import QEBase._
import Units._

case class FunType(arguments: List[Term], expr: Term){}

case class QuantityDecl(name: LocalName, arguments: List[Term], dim: Dimension){
  def toQuantity(name: LocalName) = Quantity(OMV(name), FunType(arguments, QE(dim)))
}

/*
case class Quantity(value: Term, tp: FunType) { // extends Term ?
  def times(q: Quantity) = {
    (tp.expr, q.tp.expr) match {
       case (QE(d), QE(e)) => Quantity(QMul(value, q.value), FunType(Nil, QE(DimTimes(d,e))))
       case _ => throw new scala.MatchError("Error")
    }
  }
}*/

case class Quantity(value: Term, tp: FunType) { // extends Term ?
  def times(q: Quantity) = {
    (tp.expr, q.tp.expr) match {
       case (QE(d), QE(e)) => Quantity(QMul(value, q.value), FunType(Nil, QE(DimTimes(d,e))))
       case _ => throw new scala.MatchError("Error")
    }
  }
  def div(q: Quantity) = {
    (tp.expr, q.tp.expr) match {
       case (QE(d), QE(e)) => Quantity(QDiv(value, q.value), FunType(Nil, QE(DimTimes(d,e))))
       case _ => throw new scala.MatchError("Error")
    }
  }
  def add(q: Quantity) = {
    (tp.expr, q.tp.expr) match {
       case (QE(d), QE(e)) => Quantity(QAdd(value, q.value), FunType(Nil, QE(DimTimes(d,e))))
       case _ => throw new scala.MatchError("Error")
    }
  }
  def sub(q: Quantity) = {
    (tp.expr, q.tp.expr) match {
       case (QE(d), QE(e)) => Quantity(QSub(value, q.value), FunType(Nil, QE(DimTimes(d,e))))
       case _ => throw new scala.MatchError("Error")
    }
  }
}

// A rule is an equation solved for a variable
case class Rule(solved: LocalName, solution: Term){}

// A law is a named container of all rules of an equation/formula
case class Law(name: LocalName, formula: Term, quantities: Set[LocalName], rules: Set[Rule]){
  def hasQuantity(q: QuantityDecl) = {
    var hasIt = false
    for (qname <- quantities if qname == q.name)
      hasIt = true
    hasIt
  }
}

val AccelerationDim = DimDiv(Length, DimTimes(Time, Time))

val QAcceleration = QuantityDecl(LocalName("acceleration"), Nil, AccelerationDim)


val ForceDim = DimTimes(Mass, AccelerationDim)

val QForce = QuantityDecl(LocalName("force"), Nil, ForceDim);

val QMass = QuantityDecl(LocalName("mass"), Nil, Mass);

val Newtons2ndLaw = Law(LocalName("Newtons2ndLaw"), OMA(OMID(utils.URI("path")/"id"/"of"/"law"), Nil), ["force", "mass", "acceleration"]); 


val Newtons2ndLawForceRule = Rule("force", OMA(OMID(utils.URI("path")/"id of rule term?"), Nil))

val Newtons2ndLawAccelerationRule = Rule("acceleration", OMA(OMID(utils.URI("path")/"id of rule term?"), Nil))

val Newtons2ndLawMassRule = Rule("mass", OMA(OMID(utils.URI("path")/"id of rule term?"), Nil))

// MPDs
case class MPD(quantities: Set[QuantityDecl], laws: Set[Law])
{
  var quantityConnections: Map[QuantityDecl, Set[Law]] = Map()
  var lawConnections: Map[Law, Set[QuantityDecl]] = Map()

  for (q <- quantities)
    for (r <- laws if r.hasQuantity(q))
      quantityConnections get q match {
        case None => quantityConnections = quantityConnections + (q -> Set(r))
        case Some(a) => quantityConnections = quantityConnections + (q -> (a+r))
      }

  for (r <- laws)
    for (q <- quantities if r.hasQuantity(q))
      lawConnections get r match {
        case None => lawConnections = lawConnections + (r -> Set(q))
        case Some(a) => lawConnections = lawConnections + (r -> (a+q))
      }
}



class MPDModel(mpd: MPD, unknowns: Set[LocalName], parameters: Set[LocalName])
{
  def testDeterminacy(): bool = {

  }
  def testDeterminacy(localKnowns: Set[LocalName])={
  }
}
/*

[optimisation step]
_in constructor (consider parameters unknowns):
start by going over all unknowns, and check if they're the only in the relations in their connections.

If they are, then see if can isolate the unknown quantity and make it known.

If can't isolate, `consider` it known (can pass to external solver or apply technique to compute value when parameters are given).


if they aren't, then check if there's a cycle in the graph (maze AI approach, keep history of steps and no backtracking). If there is, then can `consider` all quantities in cycle known.

[when parameters provided]
substitute values, repeat all of the above, then pass to solver as appropriate.

 */



