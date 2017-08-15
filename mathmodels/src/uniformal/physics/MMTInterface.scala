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
   
   def toFormula(t: Term): Formula = Formula(t)
   
   def toQuantity(value: Term, tp: Term): Quantity = value match {
     case QEMul(d1, d2, v1, v2) => 
       toQuantity(v1, QE(d1)).times(toQuantity(v2, QE(d2)))
     case QEDiv(d1, d2, v1, v2)  => 
       toQuantity(v1, QE(d1)).div(toQuantity(v2, QE(d2)))
     case QEAdd(d, v1, v2)  => 
       toQuantity(v1, QE(d)).add(toQuantity(v2, QE(d))) 
     case QESubtract(d, v1, v2)  => 
       toQuantity(v1, d).sub(toQuantity(v2, QE(d))) 
     case OMS(path) => 
       Quantity(value, tp)
     case x => throw new scala.MatchError("Undefined term: " + x.toString())
   }
   
   def getRules(formula: Formula): List[Rule] = {
     formula.value match {
       case Logic.and(lhs, rhs) => (getRules(toFormula(lhs)) ++ getRules(toFormula(rhs))).distinct
       case Logic._eq(tp, lhs, rhs) => {
         val solvedPath = lhs match {
           case OMS(p) => p
           case _ => throw new GeneralError("LHS of rule should be a constant symbol")
         }
         List(Rule(solvedPath, toQuantity(rhs, tp)))
       }
       case _ => throw new scala.MatchError("Error")
     }
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
                 Law(c.parent, c.name, formula, getRules(formula))
               case QE(d) =>
                 QuantityDecl(c.parent, c.name, Nil, d)
               case _ =>
                 println(c)
                 throw new scala.MatchError("Error")
             }
         }
   }
     
   def getMPDComponentsFromTheory(thy: DeclaredTheory) : List[MPDComponent] = {
     var comps: List[MPDComponent] = Nil
     thy.getDeclarations foreach {
       case c: Constant => comps ::= toMPDComponent(c)
       case Include(_, from, _) => 
         comps ++= getMPDComponentsFromTheory(controller.get(from).asInstanceOf[DeclaredTheory])
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
     if (meta.toString() != "http://mathhub.info/MitM/Models?MPD")
       return None
     
     val comps: List[MPDComponent] = getMPDComponentsFromTheory(thy)
     
     var laws: List[Law] = Nil
     var quantityDecls: List[QuantityDecl] = Nil
     comps foreach { 
       comp => comp match {
         case qd if qd.isInstanceOf[QuantityDecl]  => quantityDecls ::= qd.asInstanceOf[QuantityDecl]
         case law if law.isInstanceOf[Law] => laws ::= law.asInstanceOf[Law]
         case _ => throw new scala.MatchError("Error")
       }
     }
     Some(MPD(thy.parent, thy.name, quantityDecls, laws))
   }
}