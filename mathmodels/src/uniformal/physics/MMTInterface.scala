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
import Units.QEBase._
import Units.Field._


class MPDTool extends ShellExtension("mpd") { 
   def helpText: String = "This is the MPD Tool."

   def run(shell: Shell, args: List[String]) = {
     controller.handleLine("log console")
     controller.handleLine("server on 8080")
     val p = Path.parseM(args(0), NamespaceMap.empty)
     //println(args(0))
     
     val mpd = toMPD(p)
     println(mpd.get.prettyListCycles)
     true
   }
   
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
       case FieldEq(dim, lhs, rhs) =>{
       val solvedPath = lhs match {
           case OMS(p) => p
           case ApplyGeneral(f, x) =>
             val OMS(p) = f
             p
           case _ => throw new GeneralError("LHS of rule should be a constant symbol, not " + lhs.toString)
         }
         Formula(dim, lhs, rhs)
       }
       case _ => throw new scala.MatchError("Error")
     }
   }
   
   def toQuantity(value: Term, tp: Term): Quantity = Quantity(value, tp)
   
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
   
   def toMPDComponent(c: Constant): MPDComponent = {
         c.tp match {
           case None =>
             throw new GeneralError("No type assigned to MMT constant: " + c)
             
           case Some(t) =>
             val FunType(args, ret) = t
             ret match {
               case Logic.ded(x) =>
                 val formula = toFormula(x)
                 Law(c.parent, c.name, formula, getRules(formula), c.rl != Some("Condition"))
               
               case QE(d) =>
                 if (c.rl != None && c.rl.get != "Constant")
                   throw new GeneralError("Unknown role given to MPD quantity")
                 val df = c.df.map{t => toQuantity(t, d)}
                 QuantityDecl(c.parent, c.name, d, df, false, c.rl == Some("Constant"))
               case field(d) => {
                 if (c.rl != None && c.rl.get != "Constant")
                   throw new GeneralError("Unknown role given to MPD quantity: " + c.rl.get)
                 val df = c.df.map{t => toQuantity(t, d)}
                 QuantityDecl(c.parent, c.name, d, df, true, c.rl == Some("Constant"))
               }
               //case surface(p) =>
               //  IntegrationSurfaceDecl(c.parent, c.name)
               case space(p) => 
                 QuantitySpaceDecl(c.parent, c.name)
               case c =>
                 throw new GeneralError("Unknown construction of MPD component: " +  c.toString())
             }
         }
   }
     
   def getMPDComponentsFromTheory(thy: DeclaredTheory) : List[MPDComponent] = {
     var comps: List[MPDComponent] = Nil
     thy.getDeclarations foreach {
       case c: Constant => {println(c)
         comps ::= toMPDComponent(c)}
       case Include(_, from, _) => {
         if (from.parent.toString == "http://mathhub.info/MitM/Models")
           comps ++= getMPDComponentsFromTheory(controller.get(from).asInstanceOf[DeclaredTheory])
       }
       case _ => throw new GeneralError("Unsupported construct for mpd theory")
     }
     comps
   }
   
   def toMPD(p: MPath): Option[MPD] = {
     val thy = controller.get(p).asInstanceOf[DeclaredTheory]
     println(thy.toString)
     //None
     thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Models?MPD" => toMPD(thy)
       case _ => throw new GeneralError("Error: Theory must be a meta theory of http://mathhub.info/MitM/Models?MPD")
     }
   }
   
   def toMPD(thy: DeclaredTheory): Option[MPD] = {
     val meta = thy.meta.getOrElse(return None)
     println(meta)
     if (meta.toString() != "http://mathhub.info/MitM/Models?MPD")
       return None
     val comps: List[MPDComponent] = getMPDComponentsFromTheory(thy).reverse
     
     var laws: List[Law] = Nil
     var quantityDecls: List[QuantityDecl] = Nil
     var integrationSurfaces: List[IntegrationSurfaceDecl] = Nil
     var spaces: List[QuantitySpaceDecl] = Nil

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