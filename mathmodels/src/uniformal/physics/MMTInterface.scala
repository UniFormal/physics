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
     println(args(0))
     
     // println(thy.toString)
     
     val mpd = toMPD(p)
     //println(mpd.cycles.toString)
     println(mpd.prettyListCycles)
     true
   }
   
   def toFormula(t: Term): Formula = Formula(t)
   
   def toQuantity(value: Term, tp: Term): Quantity = Quantity(value, tp)
   
   def getRules(formula: Formula): List[Rule] = {
     formula.value match {
       case Logic.and(lhs, rhs) => (getRules(toFormula(lhs)) ++ getRules(toFormula(rhs))).distinct
       case Logic._eq(tp, lhs, rhs) => {
         val solvedPath = lhs match {
           case OMS(p) => p
           case _ => throw new scala.MatchError("Error: LHS of rule should be a constant symbol")
         }
         List(Rule(solvedPath, toQuantity(rhs, tp)))
       }
       case _ => throw new scala.MatchError("Error")
     }
   }
   
   def toMPDComponent(c: Constant): MPDComponent = {
         c.tp match {
           case None =>
             throw new scala.MatchError("Error: No type assigned to MMT constant: ", c)
             
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
       case _ => throw new scala.MatchError("Error: Unsupported construct for mpd theory")
     }
     comps
   }
   
   def toMPD(p: MPath): MPD = {
     val thy = controller.get(p).asInstanceOf[DeclaredTheory]
     toMPD(thy)
   }
   
   def toMPD(thy: DeclaredTheory): MPD = {
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
     MPD(quantityDecls, laws)
   }
}