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
import Units.FieldBase._
import Units.GeometryBase._
import Units.BoundaryConditionBase._
import Units.TacticBase._
import info.mathhub.lf.MitM.Foundation.Tensors._
import info.mathhub.lf.MitM.Foundation.RealLiterals._
import info.mathhub.lf.MitM.Foundation.NatLiterals._

abstract class ComputationElement

case class IterateOverGometry(freeVariables: List[GlobalName], variable: GlobalName, body: ComputationElement) extends ComputationElement

case class LeftAssign(freeVariables: List[GlobalName], target: GlobalName, source: GlobalName) extends ComputationElement

// case class LeftAssignStatement()

abstract class QElement
{
  def contains(e: QElement) : Boolean = {
    this match {
      case x if x == e => true
      case x: QTwoForm => x.y.contains(e) || x.x.contains(e)
      case x: QOneForm => x.x.contains(e)
      case QSymbol(_, p) => false
      case QTensorVal(_, _) => false
      case _ => throw new GeneralError("Undefined construction in search of QElement")
    }
  }
  
  def substitute(p: QSymbol, e: QElement) : QElement = {
    this match {
      case QSymbol(n, h) => if (h == p.path) e else QSymbol(n, h)
      case QMul(x, y) => QMul(x.substitute(p, e), y.substitute(p, e))
      case QDiv(x, y) => QDiv(x.substitute(p, e), y.substitute(p, e))
      case QAdd(x, y) => QAdd(x.substitute(p, e), y.substitute(p, e))
      case QSubtract(x, y) => QSubtract(x.substitute(p, e), y.substitute(p, e))
      case QNeg(x) => QNeg(x.substitute(p, e))
      case QExp(x) => QExp(x.substitute(p, e))
      case QLog(x) => QLog(x.substitute(p, e))
      case QDivergence(x) => QDivergence(x.substitute(p, e))
      case QGradient(x) => QGradient(x.substitute(p, e))
      case QTensorVal(a, b) => QTensorVal(a, b)
      case _ => throw new GeneralError("Undefined construction in subsitution of QElement" + this)
    }
  }
  
  def symbols: List[QSymbol] = {
    this match {
      case x: QTwoForm => x.y.symbols ++ x.x.symbols
      case x: QOneForm => x.x.symbols
      case QSymbol(_, p) => this.asInstanceOf[QSymbol]::Nil
      case QTensorVal(a, b) => Nil
      case _ => throw new GeneralError("Undefined construction in search of symbols: " + this.toString)
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

trait QQuantity

case class QMul(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QDiv(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QAdd(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QSubtract(override val x: QElement, override val y: QElement) extends QElement with QTwoForm
case class QNeg(override val x: QElement) extends QElement with QOneForm
case class QExp(override val x: QElement) extends QElement with QOneForm
case class QLog(override val x: QElement) extends QElement with QOneForm
case class QGradient(override val x: QElement) extends QElement with QOneForm
case class QDivergence(override val x: QElement) extends QElement with QOneForm
case class QSymbol(name: String, path: GlobalName) extends QElement with QQuantity
case class QAtSpacePoint(spacePoint: QElement, field: QElement) extends QElement with QQuantity
// case class QFieldSequence(index: QElement, 
case class QTensorVal(shape: List[Int], elems: List[String]) extends QElement
case class QGetTensorComponent(tensor: QElement, index: QElement) extends QElement

/*
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
}*/

class QuantityExpression(quantityValue: Term, expressionArgs: List[(Option[LocalName], Term)], stateless: Boolean = false) { 
      val expr: QElement = makeQuantityExpression(quantityValue, expressionArgs)
      
      private def makeTensorShape(l: Term): List[Int] = {
        def get_list_recursive(t: Term, tail: List[Int]) : List[Int] = {
          t match {
            case tcons(a, x, b) => {
              a match {
                case nat_underscore_lit(e) => get_list_recursive(b, (x.toString.toInt)::tail)
                case _ => throw new GeneralError("Unrecognized construct in tensor shape: " + a.toString)
              }
            }
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
    
    
    private def makeTensorElements(v : Term) : List[String] ={
      def get_list_recursive(t: Term) : List[String] = {
        t match {
          case r if r.getClass().getName().contains("RealLiterals") => 
            makeRealStringFromRealTerm(t)::Nil
          case nat_underscore_lit(_) => t.toString::Nil
          case start_underscore_tensor_underscore_from_underscore_number(v) => 
            get_list_recursive(v)
          case start_underscore_tensor_underscore_from_underscore_tensor(l, t, v) =>
            get_list_recursive(v)
          case append_underscore_tensor_underscore_number_underscore_component(lh, lt, t, tens, num) =>
            get_list_recursive(tens) ++ get_list_recursive(v)
          case append_underscore_tensor_underscore_tensor_underscore_component(lh, lt, t, t2, tens, tens2) =>
            get_list_recursive(tens) ++ get_list_recursive(tens2)
          case _ => throw new GeneralError("Undefined construct in tensor literal: " + t.toString)
        }
      }
      get_list_recursive(v)
    }
    
    private def makeQuantityExpression(q : Term, args: List[(Option[LocalName], Term)]): QElement = q match {
      case QuantityNeg(d, l, t, v)  => 
        QNeg(makeQuantityExpression(v, args)) 
      case FieldNeg(g, d, l, t, v) =>
        QNeg(makeQuantityExpression(v, args))
        
      /* Multiplication */
      case QuantityMul(d1, d2, l, t, v1, v2) =>
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FieldMul(d1, d2, l, t, g1, g2, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QMulF(d1, d2, l, t, g, v1, v2) =>
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FMulQ(d1, d2, l, t, g, v1, v2) =>
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QuantityLScalarMul(d1, d2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case QuantityRScalarMul(d1, d2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FieldLScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FieldRScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QLScalarMulF(d1, d2, l, t, g, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FLScalarMulQ(d1, d2, l, t, g, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QRScalarMulF(d1, d2, l, t, g, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FRScalarMulQ(d1, d2, l, t, g, v1, v2) => 
        QMul(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      
      /* Division */
      case QuantityDiv(d1, d2, l, t, v1, v2) =>
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FieldDiv(d1, d2, l, t, g1, g2, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QDivF(d1, d2, l, t, g, v1, v2) =>
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FDivQ(d1, d2, l, t, g, v1, v2) =>
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QuantityLScalarDiv(d1, d2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case QuantityRScalarDiv(d1, d2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case FieldLScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FieldRScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QLScalarDivF(d1, d2, l, t, g, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FLScalarDivQ(d1, d2, l, t, g, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
      case QRScalarDivF(d1, d2, l, t, g, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FRScalarDivQ(d1, d2, l, t, g, v1, v2) => 
        QDiv(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args))
        
      /* Addition */  
      case QuantityAdd(d, l, t, v1, v2) => 
        QAdd(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FieldAdd(d, l, t, g1, g2, v1, v2) => 
        QAdd(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case QAddF(d, l, t, g, v1, v2) => 
        QAdd(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FAddQ(d, l, t, g, v1, v2) => 
        QAdd(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      
      /* Subtraction */  
      case QuantitySubtract(d, l, t, v1, v2) => 
        QSubtract(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FieldSubtract(d, l, t, g1, g2, v1, v2) => 
        QSubtract(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case QSubtractF(d, l, t, g, v1, v2) => 
        QSubtract(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      case FSubtractQ(d, l, t, g, v1, v2) => 
        QSubtract(makeQuantityExpression(v1, args), makeQuantityExpression(v2, args)) 
      
      case QuantityExp(v) =>
        QExp(makeQuantityExpression(v, args))
        
      case QuantityLog(v) =>
        QLog(makeQuantityExpression(v, args))
        
      case FieldGradient(d, g, v) =>
        QGradient(makeQuantityExpression(v, args))
        
      case FieldDivergence(d, g, n, v) =>
        QDivergence(makeQuantityExpression(v, args)) 
        
      case MakeQuantity(l, d, v, u) =>
        QTensorVal(makeTensorShape(l), makeTensorElements(v)) 
      case MakeQuantityScalar(d, v, u) =>
        QTensorVal(List(), makeTensorElements(v)) 
        
      // Ugly use of reflection
      case r if r.getClass().getName().contains("RealLiterals") =>
        QTensorVal(List(), List(makeRealStringFromRealTerm(r)))
        
      case OMS(path) => {
        if (stateless)
          throw GeneralError("The following quantity must be stateless: " + quantityValue.toString)
        QSymbol(path.name.toString, path)
      }
      
      case OMA(OMID(path), tms) =>
        throw new GeneralError("Undefined operation: " + args.toString + tms.toString + path.toString())
        throw new GeneralError("Undefined operation: " + q.toString())
      
        
        
      case a => // I don't know how to match a literal term and OMLIT doesn't work. Temporarily, we assume everything else is a literal
        QTensorVal(List(), List(a.toString))
      
      // case t => throw new GeneralError("Couldn't match token in expression:" + t.toString())
    }
}

