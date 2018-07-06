package uniformal.physics

import info.kwarc.mmt.api._
import frontend._
import modules._
import objects._
import symbols._


import info.kwarc.mmt.lf._

import info.mathhub.lf.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QuantityBase._
import Units.LawBase._
import Units.GeometryBase._
import Units.TacticBase._
import Units.BoundaryConditionBase._


class MPDTool(controller: Controller) {    
   def toFormula(t: Term): Formula = {
     t match { 
       // TODO: let Formula take dimension instead of type
       case Logic._eq(tp, lhs, rhs) => {
         val solvedPath = lhs match {
           case OMS(p) => p
           case ApplyGeneral(f, x) =>
             val OMS(p) = f
             p
           case _ => throw new GeneralError("LHS of rule should be a constant symbol, not " + lhs.toString)
         }
         Formula(tp, lhs, rhs)
       }
       /*case FieldEq(dim, lhs, rhs) =>{
       val solvedPath = lhs match {
           case OMS(p) => p
           case ApplyGeneral(f, x) =>
             val OMS(p) = f
             p
           case _ => throw new GeneralError("LHS of rule should be a constant symbol, not " + lhs.toString)
         }
         Formula(dim, lhs, rhs)
       }*/
       case _ => throw new scala.MatchError("Error")
     }
   }
   
   def toQuantity(value: Term, tp: Term): MQuantity = MQuantity(value, tp)
   
   def getRules(formula: Formula): List[Rule] = {
         val solvedPath = formula.lhs match {
           case OMS(p) => p
           case ApplyGeneral(f, x) =>
             val OMS(p) = f
             p
           case _ => throw new GeneralError("LHS of rule should be a constant symbol")
         }
         List(Rule(solvedPath, toQuantity(formula.rhs, formula.tp))) 
   }
   
   def toEqualityLaw(parent: MPath, name: LocalName, lhs: Term, rhs: Term, dim: Term, dom: Term, rl: Option[String]) : Law = {
     val solvedPath = lhs match {
       case OMS(p) => p
       case ApplyGeneral(f, x) =>
         val OMS(p) = f
         p
       case _ => throw new GeneralError("LHS of rule should be a constant symbol, not " + lhs.toString)
     }
     val formula = Formula(dim, lhs, rhs)
     Law(parent, name, formula, getRules(formula), rl != Some("Condition"))
   }
   
   def toMPDComponent(c: Constant): Option[MPDComponent] = {
         c.tp match {
           case None =>
             throw new GeneralError("No type assigned to MMT constant: " + c)
             
           case Some(t) =>
             val FunType(args, ret) = t
             
             ret match {
               
               case Quantity(l, geom, dim, tens) =>
                 val df = c.df.map{t => toQuantity(t, ret)}
                 Some(QuantityDecl(c.parent, c.name, l, geom, dim, tens, df, false, c.rl == Some("Constant")))
              
               case geometry(p) =>
                 Some(GeometryDecl(c.parent, c.name))
               
               case EqualityLaw(g1, g2, dim, l, tens, lhs, rhs) =>
                 Some(toEqualityLaw(c.parent, c.name, lhs, rhs, dim, EVERYWHERE, c.rl))
               case EqualityLawOnDomain(g1, g2, dim, l, tens, lhs, rhs, dom) =>
                 Some(toEqualityLaw(c.parent, c.name, lhs, rhs, dim, dom, c.rl))
   
               case Logic.ded(x) =>
                 None
                 
               case Dirichlet(l, g, d, t, q, b) =>
                 println(g)
                 None
                 
               case GetStepType(a) =>
                 println("Step: ", a)
                 None
                 
               case d =>
                 throw new GeneralError("Unknown construction of MPD component: " + c.name.toString() + "..." +   d.toString())
             }
         }
   }
     
   def getMPDComponentsFromTheory(thy: DeclaredTheory) : List[MPDComponent] = {
     var comps: List[MPDComponent] = Nil
     thy.getDeclarations foreach {
       case c: Constant => {println(c)
         val comp = toMPDComponent(c)
         if (comp != None) comps ::= comp.get}
       case Include(_, from, _) => {
         if (from.parent.toString == "http://mathhub.info/MitM/Modelss")
           comps ++= getMPDComponentsFromTheory(controller.get(from).asInstanceOf[DeclaredTheory])
       }
       case _ => throw new GeneralError("Unsupported construct for mpd theory")
     }
     comps
   }
   
   def toMPD(p: MPath): Option[MPD] = {
     val thy = controller.get(p).asInstanceOf[DeclaredTheory]
     thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Models?MPD" => toMPD(thy)
       case _ => throw new GeneralError("Error: Theory must be a meta theory of http://mathhub.info/MitM/Models?MPD")
     }
   }
   
   def toMPD(thy: DeclaredTheory): Option[MPD] = {
     val meta = thy.meta.getOrElse(return None)
     println(meta)
     if (meta.toString() != "http://mathhub.info/MitM/Foundation/Units?ModelBase")
       return None
     val comps: List[MPDComponent] = getMPDComponentsFromTheory(thy).reverse
     
     var laws: List[Law] = Nil
     var quantityDecls: List[QuantityDecl] = Nil
     var integrationSurfaces: List[IntegrationSurfaceDecl] = Nil
     var spaces: List[QuantitySpaceDecl] = Nil
     var geometryDecls: List[GeometryDecl] = Nil

     comps foreach { 
       comp => comp match {
         case qd if qd.isInstanceOf[QuantityDecl]  => quantityDecls ::= qd.asInstanceOf[QuantityDecl]
         case law if law.isInstanceOf[Law] => {
           val newLaw :Law = law.asInstanceOf[Law]
           val nameSegs = newLaw.name.toString.split("_")
           if (nameSegs.last.startsWith("rule")){
             val index = laws.indexWhere{l => l.name.toString() == nameSegs.init.mkString("_")}
             if (index != -1){
               val oldl = laws(index)
               laws = laws.patch(index, Seq(Law(oldl.parent, oldl.name, oldl.formula, oldl.rules ++ newLaw.rules)), 1)
             }
           }else
             laws ::= newLaw
         }
         case geom if geom.isInstanceOf[GeometryDecl] => geometryDecls ::= geom.asInstanceOf[GeometryDecl]
         case surface if surface.isInstanceOf[IntegrationSurfaceDecl] =>
           integrationSurfaces ::= surface.asInstanceOf[IntegrationSurfaceDecl]
         
         // should deal with once I manage how to make fields with different spaces work in MMT
         case space if space.isInstanceOf[QuantitySpaceDecl] => 
           spaces ::= space.asInstanceOf[QuantitySpaceDecl]
           
         case _ => throw new scala.MatchError("Error")
       }
     }
     Some(MPD(thy.parent, thy.name, quantityDecls, laws, integrationSurfaces, spaces))
   }
}