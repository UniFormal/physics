package info.mathhub.MitM.Foundation.Units
import info.kwarc.mmt.api._
import objects._
import uom._
import ConstantScala._

/** Convenience functions for the MMT URIs of the declarations in the theory http://mathhub.info/MitM/Foundation/Units?QEBase
    along with apply/unapply methods for them */
object QEBase extends TheoryScala {
  val _base = DPath(utils.URI("http", "mathhub.info") / "MitM" / "Foundation" / "Units")
  val _name = LocalName("QEBase")

  object QEMul extends ConstantScala {
    val parent = _path
    val name = "QEMul"
    def apply(x1: Term, x2: Term, x3: Term, x4: Term) = OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil) => Some((x1, x2, x3, x4))
      case _ => None
    }
  }

  object QEDiv extends ConstantScala {
    val parent = _path
    val name = "QEDiv"
    def apply(x1: Term, x2: Term, x3: Term, x4: Term) = OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil) => Some((x1, x2, x3, x4))
      case _ => None
    }
  }

  object QEAdd extends ConstantScala {
    val parent = _path
    val name = "QEAdd"
  }

  object QESubtract extends ConstantScala {
    val parent = _path
    val name = "QESubtract"
    def apply(x1: Term, x2: Term, x3: Term) = OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil) => Some((x1, x2, x3))
      case _ => None
    }
  }

}
