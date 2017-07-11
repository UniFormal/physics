package info.mathhub.MitM.Foundation.Units
import info.kwarc.mmt.api._
import objects._
import uom._
import ConstantScala._

/** Convenience functions for the MMT URIs of the declarations in the theory http://mathhub.info/MitM/Foundation/Units?Dimensions
    along with apply/unapply methods for them */
object Dimensions extends TheoryScala {
  val _base = DPath(utils.URI("http", "mathhub.info") / "MitM" / "Foundation" / "Units")
  val _name = LocalName("Dimensions")

  object dimension extends ConstantScala {
    val parent = _path
    val name = "dimension"
  }

  object NormedDimension extends ConstantScala {
    val parent = _path
    val name = "NormedDimension"
  }

  object Length extends ConstantScala {
    val parent = _path
    val name = "Length"
  }

  object Time extends ConstantScala {
    val parent = _path
    val name = "Time"
  }

  object Mass extends ConstantScala {
    val parent = _path
    val name = "Mass"
  }

  object Amount extends ConstantScala {
    val parent = _path
    val name = "Amount"
  }

  object Temperature extends ConstantScala {
    val parent = _path
    val name = "Temperature"
  }

  object Current extends ConstantScala {
    val parent = _path
    val name = "Current"
  }

  object LuminousIntensity extends ConstantScala {
    val parent = _path
    val name = "LuminousIntensity"
  }

  object DimNone extends ConstantScala {
    val parent = _path
    val name = "DimNone"
  }

  object DimTimes extends ConstantScala {
    val parent = _path
    val name = "DimTimes"
    def apply(x1: Term, x2: Term) = OMA(OMID(this.path), x1 :: x2:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2:: scala.Nil) => Some((x1, x2))
      case _ => None
    }
  }

  object DimDiv extends ConstantScala {
    val parent = _path
    val name = "DimDiv"
    def apply(x1: Term, x2: Term) = OMA(OMID(this.path), x1 :: x2:: scala.Nil)
    def unapply(t: Term): Option[(Term, Term)] = t match {
      case OMA(OMID(this.path), x1 :: x2:: scala.Nil) => Some((x1, x2))
      case _ => None
    }
  }

}
