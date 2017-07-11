package uniformal.physics

import info.kwarc.mmt.api._
import frontend._
import modules._
import objects._
import symbols._

import info.kwarc.mmt.lf._

import info.mathhub.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QEBase._


class MPDTool extends ShellExtension("mpd") {
   def helpText: String = "This is the MPD Tool."

   def run(shell: Shell, args: List[String]) = {
     controller.handleLine("log console")
     controller.handleLine("server on 8080")

     val p = Path.parseM(args(0), NamespaceMap.empty)
     val thy = controller.get(p).asInstanceOf[DeclaredTheory]
     
     //val mpd = toMPD(thy)
     //println(mpd.toString)
     
     // turn all constants into QuantityDecls or Laws
     // then turn the whole theory into an MPD
     
     true
   }
   
   def toFormula(t: Term): Formula = ???
   def toQuantity(t: Term): Quantity = {
            t match {
               case OMS(p) => Quantity(OMS(p), ???)
               case OMV(v) =>
               case Apply(f,arg) =>
               case Lambda(x,a,t) =>
               case Pi(x,a,b) =>
               case QE(d) => Quantity(t, FunType(Nil, QE(d)))
             }
            ???
   }

   def toQuantityDecl(c: Constant): QuantityDecl = {
         c.tp match {
           case None =>
             println("no type")
           case Some(t) =>
             println(controller.presenter.asString(t))
         }
         ???
   }
   
   def toLaw(c: Constant): Law = ???
   
   def toMPD(thy: DeclaredTheory): MPD = {
     thy.getDeclarations foreach {
       case c: Constant =>
         println(c.name)
       case _ =>
     }
     ???
   }
}