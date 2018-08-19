package uniformal.physics

import info.kwarc.mmt.api._
import frontend._
import modules._
import objects._
import symbols._
import scala.util.matching.Regex

import info.kwarc.mmt.lf._

import info.mathhub.lf.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QuantityBase._
import Units.FieldBase._
import Units.LawBase._
import Units.GeometryBase._
import Units.TacticBase._
// import Units.BoundaryConditionBase._

import info.mathhub.lf.MitM.Foundation.RealLiterals._
import info.mathhub.lf.MitM.Foundation.NatLiterals._
import info.mathhub.lf.MitM.Foundation.Tensors._

import info.mathhub.lf.MitM.Foundation.Logic
import info.kwarc.mmt.odk._

import uniformal.physics.Quantities._
import uniformal.physics.Geometries._

class MPDTool(controller: Controller) {
  val ruleRegex = """^([\p{L}_]+)?(_rule(\d+))?$""".r
   
   def toQuantity(value: Term, tp: Term): MQuantity = MQuantity(value, tp)
  
   def toQStructure(value: Term, args: List[(Option[LocalName], Term)]) = MakeQuantityStructureFromTerm(value, args)
   
   def getRule(formula: Formula, ruleNumber: Option[Int], args: List[(Option[LocalName], Term)]): List[Rule] = {
         if (!formula.lhs.isInstanceOf[QSymbol])
           throw new GeneralError("LHS of rule should be a constant symbol, not " + formula.lhs.toString)
     
         List(Rule(formula.lhs.asInstanceOf[QSymbol], formula.rhs, ruleNumber)) 
   }
   
   def toStepDecl(parent: MPath, name: LocalName, tp: Term): StepDecl = {
     def getStepPairList(t: Term): List[(GlobalName, GlobalName)] =
     t match {
      // case StepAggregate(agg, MakeStep(OMS(quantityPath), OMS(lawPath))) => (quantityPath, lawPath)::getStepPairList(agg)
      // case MakeStep(OMS(quantityPath), OMS(lawPath)) => (quantityPath, lawPath)::Nil
       case _ => {
         throw new GeneralError("Unidentified component in step: " + t.toString)
       }
     }
     StepDecl(parent, name, getStepPairList(tp))
   }
   
   def toEqualityLaw(parent: MPath, name: LocalName, lhs: Term, rhs: Term, tp: Term, args: List[(Option[LocalName], Term)], rl: Option[String]) : Law = {  
     val formula = Formula(MakeQuantityStructureFromTerm(lhs, args),
                           MakeQuantityStructureFromTerm(rhs, args), args)
     
     val (lawName, ruleNumber) = getRuleNameData(name.toString)
     
     // TODO: ensure that further possible rules are generated from additional rules as well
     if (ruleNumber != None)
       Law(parent, LocalName.parse(lawName), formula, getRule(formula, ruleNumber, args), rl != Some("Condition"))
     else
       Law(parent, LocalName.parse(lawName), formula, List(), rl != Some("Condition"))

   }
   
   def getRuleNameData(name: String): (String, Option[Int]) = {
     ruleRegex.findAllIn(name).matchData.map({
       m: Regex.Match => {
         val baseName = m.group(1)
         val ruleNumber = m.group(3) match {
           case null => None
           case a: String => Some(a.toInt) 
         }
         (baseName, ruleNumber)
       }
     }).toList(0)
   }
   
   private def makeListFromListTerm(l: Term): List[String] = {
    def get_list_recursive(t: Term, tail: List[String]) : List[String] = {
      t match {
        case tcons(a, x, b) => {
          a match {
            case nat_underscore_lit(e) => get_list_recursive(b, (x.toString)::tail)
          }
        }
        case nnil(p) => return tail
      }
    }
    get_list_recursive(l, Nil)
  }
   
   private def getTensorRankShape(tensorType: Term) : List[Int] = {
     tensorType match {
       case tensor_underscore_type(l) => {
         makeListFromListTerm(l).map(n => n.toInt)
       }
       case Tensors.Scalar(l) => List()
       case Tensors.Vector(i) => List(i.toString.toInt)
       case Tensors.Matrix(j,k) => List(j.toString.toInt, k.toString.toInt)
       case _ => throw new GeneralError("Can't recognize tensor type: " + tensorType.toString)
     }
   }
   
   def toRuleComponent(c: Constant, x: Term, args: List[(Option[LocalName], Term)]) = {
     x match {
       case Logic._eq(tp, lhs, rhs) if c.rl.get.contains("Law") => {
         Some(toEqualityLaw(c.parent, c.name, lhs, rhs, tp, args, c.rl))
       }
       //case x if c.rl.get.contains("ComputationStep") => 
       //  throw new GeneralError(args)
       case _ => None
     }
   }
   
   def toMPDComponent(c: Constant): Option[MPDComponent] = {
         c.tp match {
           case None =>
             c.df match {
               case None => None
               
               case Some(df) => df match {
                 case x => throw new GeneralError("Definition encountered: " + df.toString)
               }
             }
             //throw new GeneralError("No type assigned to MMT constant: " + c)
             
           case Some(t) =>
             val FunType(args, ret) = t
             ret match {
               case Quantity(l, dim, tens) =>
                 val df = c.df.map{t => toQuantity(t, ret)}
                 Some(QuantityDecl(c.parent, c.name, l, None, dim, getTensorRankShape(tens), df,
                     false,
                     false, 
                     c.rl != None && c.rl.get.contains("Constant")))
              
               case Arrow(g, Quantity(l, dim, tens)) => 
                 val df = c.df.map{t => toQuantity(t, ret)}
                 Some(QuantityDecl(c.parent, c.name, l, Some(MakeGeometryStructureFromTerm(g)), dim, getTensorRankShape(tens), df,
                     false,
                     true, 
                     c.rl != None && c.rl.get.contains("Constant")))    
                 
               case Univ(1) => {/* Univ(ktype) = Univ(1) = type */
                 if (c.df != None) {
                   c.df.get match {
                     case construct_underscore_geometry(f) =>
                       Some(GeometryDecl(c.parent, c.name, Some(GConstructionFromDescription("Temp"))))
                     case make_underscore_discrete_underscore_geometry(t) =>
                       None
                     case _ => throw new GeneralError("Undefined definition to oftype object: " + c.df.get.toString)
                   }
                 } else {
                   Some(GeometryDecl(c.parent, c.name, None))
                 }
               }
               
               case sequence_underscore_type(make_underscore_discrete(args)) =>                 
                 Some(QuantitySequenceDecl(args._5.toMPath.^, args._5.toMPath.toGlobalName.name))
               
               case make_underscore_discrete(args) =>
                 None
                 
               case subtypeJudg(t1, t2) =>
                 None
                 
               case Logic.ded(x) =>
                 x match {
                   case Logic._eq(tp, lhs, rhs) if c.rl.get.contains("Law") => {
                     Some(toEqualityLaw(c.parent, c.name, lhs, rhs, tp, args, c.rl))
                   }
                   case x if c.rl.get.contains("ComputationStep") => 
                     throw new GeneralError(args.toString())
                   case _ => None
                 }
                 
               case Pi(name, tp, body) =>
                 throw new GeneralError("hodgepodge")
                 
               case Dirichlet(l, g, d, t, q, b) =>
                 println(g)
                 None
                 
              // case GetStepType(a) =>
              //   Some(toStepDecl(c.parent, c.name, a))
                 
               case d =>
                 throw new GeneralError("Unknown construction of MPD component: " + c.name.toString + "..." +  d.toString())
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
         if (from.parent.toString == "http://mathhub.info/MitM/Models")
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
     var computationSteps: List[BigStep] = Nil
     
     comps foreach { 
       comp => comp match {
         case qd if qd.isInstanceOf[QuantityDecl]  => quantityDecls ::= qd.asInstanceOf[QuantityDecl]
         case law if law.isInstanceOf[Law] => {
           val newLaw :Law = law.asInstanceOf[Law]
           val (lawName, ruleNumber) = getRuleNameData(newLaw.name.toString)
           val index = laws.indexWhere{l => l.name.toString() == lawName}
           if (index != -1){
             val oldl = laws(index)
             laws = laws.patch(index, Seq(Law(oldl.parent, oldl.name, oldl.formula, oldl.rules ++ newLaw.rules)), 1)
           } else {
             laws ::= newLaw
           }
         }
         case stepDecl if stepDecl.isInstanceOf[StepDecl] => {
           val newStepDecl = stepDecl.asInstanceOf[StepDecl]
           val bigStep = BigStep(newStepDecl.quantityLawPairPaths.map(t => 
             t match{     
               case (qP, lP) =>
               {
                   val quantityDecl = quantityDecls.find(p => p.path == qP)
                   val law = laws.find(l => l.path == lP)
                   if (quantityDecl == None) 
                     throw new GeneralError("Can't find quantity decl for step: " + qP.toString)
                   if (law == None) 
                     throw new GeneralError("Can't find law for step: " + lP.toString)
                   Step(law.get, quantityDecl.get)
               }
             }
           ), Some(newStepDecl.path))
           if (!bigStep.isConnected)
             throw new GeneralError("Computation step is not connected: " + newStepDecl.path.toString)
           computationSteps ::= bigStep
           println("Hodge: " + bigStep.isConnected, bigStep.steps.map(x => (x.law.name, x.quantityDecl.name)))
         }
         
         case geom if geom.isInstanceOf[GeometryDecl] => geometryDecls ::= geom.asInstanceOf[GeometryDecl]
         case surface if surface.isInstanceOf[IntegrationSurfaceDecl] =>
           integrationSurfaces ::= surface.asInstanceOf[IntegrationSurfaceDecl]
         
         // should deal with once I manage how to make fields with different spaces work in MMT
         case space if space.isInstanceOf[QuantitySpaceDecl] => 
           spaces ::= space.asInstanceOf[QuantitySpaceDecl]
           
         case seq if seq.isInstanceOf[QuantitySequenceDecl] =>
           val newSeq = seq.asInstanceOf[QuantitySequenceDecl]
           val quantitySeqIndex = quantityDecls.indexWhere(x => (x.parent ? x.name) == (newSeq.quantityParent ? newSeq.quantityName))
           if (quantitySeqIndex == -1)
             throw GeneralError("Can't find quantity referred to in sequence: " + newSeq)
           quantityDecls = quantityDecls.updated(quantitySeqIndex, QuantityDecl(quantityDecls(quantitySeqIndex).parent,
                                  quantityDecls(quantitySeqIndex).name,
                                  quantityDecls(quantitySeqIndex).l,
                                  quantityDecls(quantitySeqIndex).geom,
                                  quantityDecls(quantitySeqIndex).dim,
                                  quantityDecls(quantitySeqIndex).tensRank,
                                  quantityDecls(quantitySeqIndex).df,
                                  true,
                                  quantityDecls(quantitySeqIndex).isField,
                                  quantityDecls(quantitySeqIndex).isConstant))
                            
           
         case _ => throw new GeneralError("MPD component not handled")
       }
     }
     Some(MPD(thy.parent, thy.name, quantityDecls, laws, computationSteps, integrationSurfaces, spaces))
   }
}