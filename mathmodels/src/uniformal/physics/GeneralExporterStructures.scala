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
{
  def contains(e: QElement) : Boolean = {
    e match {
      case x if x == e => true
      case x: QTwoForm => x.y.contains(e) && x.x.contains(e)
      case x: QOneForm => x.x.contains(e)
      case QSymbol(_, p) => false
      case QTensorVal(_, _) => false
      case _ => throw new GeneralError("Undefined construction in search of QElement")
    }
  }
  
  def substitute(p: QSymbol, e: QElement) : QElement = {
    this match {
      case x: QSymbol if x == p => e
      case x => x
      // TODO: needs more cases
      case _ => throw new GeneralError("Undefined construction in subsitution of QElement")
    }
  }
}


trait QTwoForm {
  val x: QElement
  val y: QElement
  
}

trait QOneForm {
  val x: QElement
}

case class QMul(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QDiv(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QAdd(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QSubtract(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QNeg(override val x: QElement) extends QElement with QOneForm
case class QExp(override val x: QElement) extends QElement with QOneForm
case class QLog(override val x: QElement) extends QElement with QOneForm
case class QGradient(override val x: QElement) extends QElement with QOneForm
case class QDivergence(override val x: QElement) extends QElement with QOneForm
case class QTensorVal(t: Term, tr: Term) extends QElement
case class QSymbol(name: String, path: GlobalName) extends QElement

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

class QuantityExpression(quantityValue: Term, stateless: Boolean = false) {
    
    val expr: QElement = makeQuantityExpression(quantityValue)
    
    private def makeQuantityExpression(q : Term): QElement = q match {
      case QuantityNeg(d, g, l, t, v) => 
        QNeg(makeQuantityExpression(v)) 
     
      case QuantityMul(d1, d2, g1, g2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1), makeQuantityExpression(v2)) 
      case QuantityLScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1), makeQuantityExpression(v2)) 
      case QuantityRScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1), makeQuantityExpression(v2))
      
      case QuantityDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2)) 
      case QuantityLScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2)) 
      case QuantityRScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1), makeQuantityExpression(v2))
       
      case QuantityAdd(d, l, t, g1, g2, v1, v2) => 
        QAdd(makeQuantityExpression(v1), makeQuantityExpression(v2)) 
      
      case QuantitySubtract(d, g1, g2, l, t, v1, v2) => 
        QSubtract(makeQuantityExpression(v1), makeQuantityExpression(v2))
        
      case QuantityExp(g, v) =>
        QExp(makeQuantityExpression(v))
        
      case QuantityLog(g, v) =>
        QLog(makeQuantityExpression(v))
        
      case QuantityGradient(d, g, v) =>
        QGradient(makeQuantityExpression(v))
        
      case QuantityDivergence(d, g, n, v) =>
        QDivergence(makeQuantityExpression(v)) 
        
      case MakeQuantity(l, g, d, t, lr) =>
        QTensorVal(l, lr) 
      case MakeQuantityOnGeometry(l, g, d, t, lr) =>
        QTensorVal(l, lr) 
        
      case OMS(path) => {
        if (stateless)
          throw GeneralError("The following quantity must be stateless: " + quantityValue.toString)
        QSymbol(path.name.toString, path)
      }
      
      case OMA(OMID(path), _) =>
        throw new GeneralError("Undefined operation: " + q.toString())
      case t => throw new GeneralError("Match Error:" + t.toStr(true))
    }
}