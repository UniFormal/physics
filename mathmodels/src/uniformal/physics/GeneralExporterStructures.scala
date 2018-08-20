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

object Booleans {
  abstract class BStructure
  
  case class BNot(val x: BStructure) extends BStructure
  case class BAnd(val x: BStructure, val y: BStructure) extends BStructure
  case class BOr(val x: BStructure, val y: BStructure) extends BStructure
  case class BEqual(val x: Quantities.QStructure, val y: Quantities.QStructure) extends BStructure
  case class BGreaterThanOrEqual(val x: Quantities.QStructure, val y: Quantities.QStructure) extends BStructure
  case class BLessThanOrEqual(val x: Quantities.QStructure, val y: Quantities.QStructure) extends BStructure
  
  def MakeBooleanStructureFromTerm(p : Term, args: List[(Option[LocalName], Term)] = Nil): BStructure = p match {
    case Logic.and(x, y) =>
      BAnd(MakeBooleanStructureFromTerm(x), 
           MakeBooleanStructureFromTerm(y))
           
    case Logic.or(x, y) =>
      BOr(MakeBooleanStructureFromTerm(x), 
           MakeBooleanStructureFromTerm(y))
  
    case Logic._eq(t, x, y) => 
      BEqual(Quantities.MakeQuantityStructureFromTerm(x, args), 
             Quantities.MakeQuantityStructureFromTerm(y, args))
             
    case leq_underscore_real_underscore_lit(x, y) =>
      BLessThanOrEqual(Quantities.MakeQuantityStructureFromTerm(x, args),
                       Quantities.MakeQuantityStructureFromTerm(y, args))
                
    case Logic.neq(t, x, y) =>
      BNot(BEqual(Quantities.MakeQuantityStructureFromTerm(x, args), 
                  Quantities.MakeQuantityStructureFromTerm(y, args)))
                  
    case Logic.not(x) =>
      BNot(MakeBooleanStructureFromTerm(x))
      
    case _ =>   
      throw new GeneralError("Couldn't match token in predicate expression:" + p.toString())
  }
}

object ComputationalSteps {
  abstract class CStructure
  
  case class IterateOverGometry(freeVariables: List[GlobalName], variable: GlobalName, body: CStructure) extends CStructure
  
  case class LeftAssign(freeVariables: List[GlobalName], target: GlobalName, source: GlobalName) extends CStructure
  
  // case class LeftAssignStatement()
}
  
object Geometries {
  abstract class GStructure
  
  trait GElement
  
  case class GBoundary(val x: GStructure) extends GStructure
  case class GComplement(val x: GStructure) extends GStructure
  case class GUnion(val x: GStructure, val y: GStructure) extends GStructure
  case class GIntersection(val x: GStructure, val y: GStructure) extends GStructure
  case class GSymbol(name: String, path: GlobalName) extends GStructure with GElement
  case class GConstructionFromPredicate(val f: Booleans.BStructure, val args: List[(LocalName, Term)]) extends GStructure with GElement
  case class GConstructionFromDescription(val s: String) extends GStructure with GElement

  def MakeGeometryStructureFromTerm(g : Term): GStructure = g match {
    case GeometryBoundary(g1)  =>
      GBoundary(MakeGeometryStructureFromTerm(g1))
      
    case GeometryComplement(g1)  =>
      GComplement(MakeGeometryStructureFromTerm(g1))
      
    case GeometryUnion(g1, g2)  =>
      GUnion(MakeGeometryStructureFromTerm(g1), MakeGeometryStructureFromTerm(g2))
      
    case GeometryIntersection(g1, g2)  =>
      GUnion(MakeGeometryStructureFromTerm(g1), MakeGeometryStructureFromTerm(g2))
      
    case OMS(path) =>
      GSymbol(path.name.toString, path)
      
    case _ =>   
      throw new GeneralError("Couldn't match token in geometry expression:" + g.toString())   
  }  
}

object Quantities {
  abstract class QStructure
  {
    def isStateless = symbols.isEmpty
    
    def contains(e: QStructure) : Boolean = {
      this match {
        case x if x == e => true
        case x: QTwoForm => x.y.contains(e) || x.x.contains(e)
        case x: QOneForm => x.x.contains(e)
        case QSymbol(_, p, false) => false
        case QTensorVal(_, _) => false
        case _ => throw new GeneralError("Undefined construction in search of QStructure")
      }
    }
    
    def substitute(p: QSymbol, e: QStructure) : QStructure = {
      this match {
        case QSymbol(n, h, false) => if (h == p.path) e else QSymbol(n, h)
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
        case _ => throw new GeneralError("Undefined construction in subsitution of QStructure" + this)
      }
    }
    
    def symbols: List[QSymbol] = {
      this match {
        case x: QTwoForm => x.y.symbols ++ x.x.symbols
        case x: QOneForm => x.x.symbols
        case QSymbol(_, p, false) => this.asInstanceOf[QSymbol]::Nil
        case QTensorVal(a, b) => Nil
        case _ => throw new GeneralError("Undefined construction in search of symbols: " + this.toString)
      }
    }
  }
  
  trait QTwoForm {
    val x: QStructure
    val y: QStructure
  }
  
  trait QOneForm {
    val x: QStructure
  }
  
  trait QElement
  
  case class QMul(override val x: QStructure, override val y: QStructure) extends QStructure with QTwoForm
  case class QDiv(override val x: QStructure, override val y: QStructure) extends QStructure with QTwoForm
  case class QAdd(override val x: QStructure, override val y: QStructure) extends QStructure with QTwoForm
  case class QSubtract(override val x: QStructure, override val y: QStructure) extends QStructure with QTwoForm
  case class QNeg(override val x: QStructure) extends QStructure with QOneForm
  case class QAbs(override val x: QStructure) extends QStructure with QOneForm
  case class QExp(override val x: QStructure) extends QStructure with QOneForm
  case class QLog(override val x: QStructure) extends QStructure with QOneForm
  case class QGradient(override val x: QStructure) extends QStructure with QOneForm
  case class QDivergence(override val x: QStructure) extends QStructure with QOneForm
  case class QSymbol(name: String, path: GlobalName, isArg: Boolean = false) extends QStructure with QElement
  case class QAtSpacePoint(spacePoint: QStructure, field: QStructure) extends QStructure with QElement
  // case class QFieldSequence(index: QElement, 
  case class QTensorVal(shape: List[Int], elems: List[String]) extends QStructure
  case class QGetTensorComponent(tensor: QStructure, index: QStructure) extends QStructure
  
  
  private def MakeTensorShapeFromTerm(l: Term): List[Int] = {
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
  
  private def MakeRealStringFromRealTerm(r:Term): String = {
    r match {
      case real_underscore_times_underscore_tenth_underscore_pow(a, b) => 
        (MakeRealStringFromRealTerm(a).toDouble * scala.math.pow(10.0,
                       MakeRealStringFromRealTerm(b).toDouble)).toString
      case times_underscore_real_underscore_lit(a, b) =>
        (MakeRealStringFromRealTerm(a).toDouble *
         MakeRealStringFromRealTerm(b).toDouble).toString
      case minus_underscore_real_underscore_lit(a) =>
        (- MakeRealStringFromRealTerm(a).toDouble).toString
      case a => a.toString
    }
  }
  
  
  private def MakeTensorElementsFromTerm(v : Term) : List[String] ={
    def get_list_recursive(t: Term) : List[String] = {
      t match {
        case r if r.getClass().getName().contains("RealLiterals") => 
          MakeRealStringFromRealTerm(t)::Nil
        case nat_underscore_lit(_) => t.toString::Nil
        case start_underscore_tensor_underscore_from_underscore_number(v) => 
          get_list_recursive(v)
        case start_underscore_tensor_underscore_from_underscore_tensor(l, t, v) =>
          get_list_recursive(v)
        case append_underscore_tensor_underscore_number_underscore_component(lh, lt, t, tens, num) =>
          get_list_recursive(tens) ++ get_list_recursive(v)
        case append_underscore_tensor_underscore_tensor_underscore_component(lh, lt, t, t2, tens, tens2) =>
          get_list_recursive(tens) ++ get_list_recursive(tens2)
        case _ => 
          try {
            List(t.toString.toDouble.toString)
          } catch {
            case _ : Throwable => throw new GeneralError("Undefined construct in tensor literal: " + t.toString)
          }
      }
    }
    get_list_recursive(v)
  }
  
  def MakeQuantityStructureFromTerm(q : Term, args: List[(Option[LocalName], Term)], stateless: Boolean = false): QStructure = q match {
    case QuantityNeg(d, l, t, v)  => 
      QNeg(MakeQuantityStructureFromTerm(v, args)) 
    case FieldNeg(g, d, l, t, v) =>
      QNeg(MakeQuantityStructureFromTerm(v, args))
      
    /* Multiplication */
    case QuantityMul(d1, d2, l, t, v1, v2) =>
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FieldMul(d1, d2, l, t, g1, g2, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QMulF(d1, d2, l, t, g, v1, v2) =>
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FMulQ(d1, d2, l, t, g, v1, v2) =>
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QuantityLScalarMul(d1, d2, l, t, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case QuantityRScalarMul(d1, d2, l, t, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FieldLScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FieldRScalarMul(d1, d2, g1, g2, l, t, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QLScalarMulF(d1, d2, l, t, g, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FLScalarMulQ(d1, d2, l, t, g, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QRScalarMulF(d1, d2, l, t, g, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FRScalarMulQ(d1, d2, l, t, g, v1, v2) => 
      QMul(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    
    /* Division */
    case QuantityDiv(d1, d2, l, t, v1, v2) =>
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FieldDiv(d1, d2, l, t, g1, g2, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QDivF(d1, d2, l, t, g, v1, v2) =>
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FDivQ(d1, d2, l, t, g, v1, v2) =>
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QuantityLScalarDiv(d1, d2, l, t, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case QuantityRScalarDiv(d1, d2, l, t, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case FieldLScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FieldRScalarDiv(d1, d2, g1, g2, l, t, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QLScalarDivF(d1, d2, l, t, g, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FLScalarDivQ(d1, d2, l, t, g, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
    case QRScalarDivF(d1, d2, l, t, g, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FRScalarDivQ(d1, d2, l, t, g, v1, v2) => 
      QDiv(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args))
      
    /* Addition */  
    case QuantityAdd(d, l, t, v1, v2) => 
      QAdd(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FieldAdd(d, l, t, g1, g2, v1, v2) => 
      QAdd(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case QAddF(d, l, t, g, v1, v2) => 
      QAdd(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FAddQ(d, l, t, g, v1, v2) => 
      QAdd(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    
    /* Subtraction */  
    case QuantitySubtract(d, l, t, v1, v2) => 
      QSubtract(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FieldSubtract(d, l, t, g1, g2, v1, v2) => 
      QSubtract(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case QSubtractF(d, l, t, g, v1, v2) => 
      QSubtract(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    case FSubtractQ(d, l, t, g, v1, v2) => 
      QSubtract(MakeQuantityStructureFromTerm(v1, args), MakeQuantityStructureFromTerm(v2, args)) 
    
    case QuantityExp(v) =>
      QExp(MakeQuantityStructureFromTerm(v, args))
      
    case QuantityLog(v) =>
      QLog(MakeQuantityStructureFromTerm(v, args))
      
    case FieldGradient(d, g, v) =>
      QGradient(MakeQuantityStructureFromTerm(v, args))
      
    case FieldDivergence(d, g, n, v) =>
      QDivergence(MakeQuantityStructureFromTerm(v, args)) 
      
    case MakeQuantity(l, d, v, u) =>
      QTensorVal(MakeTensorShapeFromTerm(l), MakeTensorElementsFromTerm(v)) 
    case MakeQuantityScalar(d, v, u) =>
      QTensorVal(Nil, MakeTensorElementsFromTerm(v)) 
  
    case StripTensorFromQuantity(l, d, t, v) =>
      MakeQuantityStructureFromTerm(v, args)
      
    case get_underscore_vector_underscore_component(h, v, index) =>
      QGetTensorComponent(MakeQuantityStructureFromTerm(v, args), MakeQuantityStructureFromTerm(index, args))
    
    case real_underscore_abs(r) =>
      QAbs(MakeQuantityStructureFromTerm(r, args))
      
    // Ugly use of reflection
    case r if r.getClass().getName().contains("RealLiterals") =>
      QTensorVal(Nil, List(MakeRealStringFromRealTerm(r)))
      
    case OMS(path) => {
      if (stateless)
        throw GeneralError("The following quantity must be stateless: " + q.toString)
      QSymbol(path.name.toString, path)
    }
    
    case OMA(OMID(path), tms) =>
      throw new GeneralError("Undefined operation: " + args.toString + tms.toString + path.toString())
      throw new GeneralError("Undefined operation: " + q.toString())
    
      
      
    case a => // I don't know how to match a literal term and OMLIT doesn't work. Temporarily, we assume everything else is a literal
      QTensorVal(Nil, List(a.toString))
    
    // case t => throw new GeneralError("Couldn't match token in quantity expression:" + t.toString())
  }
}  


