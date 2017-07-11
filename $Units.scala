package info.mathhub.MitM.Foundation.Units
import info.kwarc.mmt.api._
import objects._
import uom._
import ConstantScala._

/** Convenience functions for the MMT URIs of the declarations in the theory http://mathhub.info/MitM/Foundation/Units?Units
    along with apply/unapply methods for them */
object Units extends TheoryScala {
  val _base = DPath(utils.URI("http", "mathhub.info") / "MitM" / "Foundation" / "Units")
  val _name = LocalName("Units")

  object unit extends ConstantScala {
    val parent = _path
    val name = "unit"
  }

  object QE extends ConstantScala {
    val parent = _path
    val name = "QE"
  }

  object NoneIsReal extends ConstantScala {
    val parent = _path
    val name = "NoneIsReal"
  }

  object unitappl extends ConstantScala {
    val parent = _path
    val name = "unitappl"
    def apply(x1: Term, x2: Term, x3: Term) = OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil) => Some((x1, x2, x3))
      case _ => None
    }
  }

  object scalar extends ConstantScala {
    val parent = _path
    val name = "scalar"
    def apply(x1: Term, x2: Term) = OMA(OMID(this.path), x1 :: x2:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2:: scala.Nil) => Some((x1, x2))
      case _ => None
    }
  }

  object scalar_underscore_commutes extends ConstantScala {
    val parent = _path
    val name = "scalar_commutes"
    def apply(x1: Term, x2: Term, x3: Term) = OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil) => Some((x1, x2, x3))
      case _ => None
    }
  }

  object unitof extends ConstantScala {
    val parent = _path
    val name = "unitof"
    def apply(x1: Term, x2: Term) = OMA(OMID(this.path), x1 :: x2:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2:: scala.Nil) => Some((x1, x2))
      case _ => None
    }
  }

  object unitof_underscore_commutes extends ConstantScala {
    val parent = _path
    val name = "unitof_commutes"
    def apply(x1: Term, x2: Term, x3: Term) = OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3:: scala.Nil) => Some((x1, x2, x3))
      case _ => None
    }
  }

  object lift extends ConstantScala {
    val parent = _path
    val name = "lift"
    def apply(x1: Term, x2: Term, x3: Term, x4: Term) = OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil) => Some((x1, x2, x3, x4))
      case _ => None
    }
  }

  object UnitTimes extends ConstantScala {
    val parent = _path
    val name = "UnitTimes"
    def apply(x1: Term, x2: Term, x3: Term, x4: Term) = OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil) => Some((x1, x2, x3, x4))
      case _ => None
    }
  }

  object UnitDiv extends ConstantScala {
    val parent = _path
    val name = "UnitDiv"
    def apply(x1: Term, x2: Term, x3: Term, x4: Term) = OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term, Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2 :: x3 :: x4:: scala.Nil) => Some((x1, x2, x3, x4))
      case _ => None
    }
  }

}
