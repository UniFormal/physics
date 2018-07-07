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
import Units.GeometryBase._
import Units.BoundaryConditionBase._
import Units.TacticBase._

abstract class QElement

case class QMul(x: String, y: String) extends QElement
case class QDiv(x: String, y: String) extends QElement
case class QAdd(x: String, y: String) extends QElement
case class QSubtract(x: String, y: String) extends QElement
case class QNeg(x: String) extends QElement
case class QExp(x: String) extends QElement
case class QLog(x: String) extends QElement
case class QGradient(x: String) extends QElement
case class QDivergence(x: String) extends QElement
case class QTensorVal(t: Term, tr: Term) extends QElement
case class QSymbol(x: String, state: String) extends QElement

object MQuantityMul
{
  def unapply(q :MQuantity): Option[(MQuantity, MQuantity)] = q.tp match {
    case QuantityMul(d1, d2, g1, g2, l, t, v1, v2) =>
      Some(MQuantity(v1, Quantity(l, g1, d1, t), false), 
          MQuantity(v2, Quantity(l, g2, d2, t), false))
    case _ => None
  }
}

object MQuantityDiv
{
  def unapply(q :MQuantity): Option[(MQuantity, MQuantity)] = q.tp match {
    case QuantityDiv(d1, d2, g1, g2, l, t, v1, v2) =>
      Some(MQuantity(v1, Quantity(l, g1, d1, t), false), 
          MQuantity(v2, Quantity(l, g2, d2, t), false))
    case _ => None
  }
}

object MQuantityAdd
{
  def unapply(q :MQuantity): Option[(MQuantity, MQuantity)] = q.tp match {
    case QuantityAdd(d, l, t, g1, g2, v1, v2) =>
      Some(MQuantity(v1, Quantity(l, g1, d, t), false), 
          MQuantity(v2, Quantity(l, g2, d, t), false))
    case _ => None
  }
}

object MQuantitySubtract
{
  def unapply(q :MQuantity): Option[(MQuantity, MQuantity)] = q.tp match {
    case QuantityAdd(d, l, t, g1, g2, v1, v2) =>
      Some(MQuantity(v1, Quantity(l, g1, d, t), false), 
          MQuantity(v2, Quantity(l, g2, d, t), false))
    case _ => None
  }
}

class QuantityStringExpression(f: (QElement => String), state: String, quantityValue: Term, mustBeStateless: Boolean = false) {
    var parameters: List[String] = Nil
     
    val expression: String = makeQuantityExpression(quantityValue)
    
    private def makeQuantityExpression(q : Term): String = q match {
      case QuantityNeg(d, g, l, t, v) => 
        f(QNeg(makeQuantityExpression(v))) 
     
      case QuantityMul(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QMul(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
      case QuantityLScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QMul(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
      case QuantityRScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QMul(makeQuantityExpression(v1), makeQuantityExpression(v2)))
      
      case QuantityDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
      case QuantityLScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
      case QuantityRScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        f(QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
       
      case QuantityAdd(d, l, t, g1, g2, v1, v2) => 
        f(QAdd(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
      
      case QuantitySubtract(d, g1, g2, l, t, v1, v2) => 
        f(QSubtract(makeQuantityExpression(v1), makeQuantityExpression(v2))) 
        
      case QuantityExp(g, v) =>
        f(QExp(makeQuantityExpression(v))) 
        
      case QuantityLog(g, v) =>
        f(QLog(makeQuantityExpression(v))) 
        
      case QuantityGradient(d, g, v) =>
        f(QGradient(makeQuantityExpression(v))) 
        
      case QuantityDivergence(d, g, n, v) =>
        f(QDivergence(makeQuantityExpression(v))) 
        
      case MakeQuantity(l, g, d, t, lr) =>
        f(QTensorVal(l, lr)) 
      case MakeQuantityOnGeometry(l, g, d, t, lr) =>
        f(QTensorVal(l, lr)) 
        
      case OMS(path) => {
        if (mustBeStateless)
          throw GeneralError("The following quantity must be stateless: " + quantityValue.toString)
        parameters ::= path.name.toString
        f(QSymbol(path.name.toString, state))
      }
      
      case OMA(OMID(path), _) =>
        throw new GeneralError("Undefined operation: " + q.toString())
      case t => throw new GeneralError("Match Error:" + t.toStr(true))
    }
}