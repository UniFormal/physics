package uniformal.physics

import info.kwarc.mmt.api._
import objects._

import info.mathhub.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QEBase._

object Test {

  val ns = Units.Units._base / "test"
  val mpath = ns ? "Test"
  
  val AccelerationDim = DimDiv(Length, DimTimes(Time, Time))
  
  val QAcceleration = QuantityDecl(mpath, LocalName("acceleration"), Nil, AccelerationDim)
  
  
  val ForceDim = DimTimes(Mass, AccelerationDim)
  
  val QForce = QuantityDecl(mpath, LocalName("force"), Nil, ForceDim)
  
  val QMass = QuantityDecl(mpath, LocalName("mass"), Nil, Mass)
    
  val Newtons2ndLawForceRule = Rule(QForce.path, QMass.toQuantity times QAcceleration.toQuantity)
  val Newtons2ndLawAccelerationRule = Rule(QAcceleration.path, QForce.toQuantity div QMass.toQuantity)
  val Newtons2ndLawMassRule = Rule(QMass.path, QForce.toQuantity div QAcceleration.toQuantity)
  val NewtonRules = Set(Newtons2ndLawForceRule,Newtons2ndLawAccelerationRule,Newtons2ndLawMassRule)
  val Newtons2ndLaw = Law(mpath, LocalName("Newtons2ndLaw"), 
      QForce.toQuantity equal (QMass.toQuantity times QAcceleration.toQuantity), NewtonRules.toList)
}
