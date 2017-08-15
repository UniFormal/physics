package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._

import objects._

import info.mathhub.lf.MitM.Foundation._
import Units.Units._
import Units.Dimensions._
import Units.QEBase._

/**
 * run via build mpd-python ARCHIVE
 */
class PythonExporter extends Exporter {
  
  val key = "mpd-python"
  private lazy val mpdtool = {
    val m = new MPDTool
    initOther(m)
    m
  }
  
  private[this] def pyIndent(n: Int): String = "\t"*n
  
  private[this] def ensureIdentifierString(s :String): String = {
    var out: String = s
    if (s.headOption.getOrElse(return s).isDigit)
      out = "_"+out
    out
  }
  
  private[this] class QuantityPyExpression(quantity: Quantity) {
    var parameters: List[String] = Nil
    
    val expression = makeQuantityPyExpression(quantity)
    
    private def makeQuantityPyExpression(q : Quantity): String = q match { 
      case Quantity(QEMul(d1, d2, v1, v2), ft) => 
        s"(${makeQuantityPyExpression(Quantity(v1, ft))} * ${makeQuantityPyExpression(Quantity(v2, ft))})" 
      case Quantity(QEDiv(d1, d2, v1, v2), ft) => 
        s"(${makeQuantityPyExpression(Quantity(v1, ft))} / ${makeQuantityPyExpression(Quantity(v2, ft))})" 
      case Quantity(QEAdd(d, v1, v2), ft) => 
        s"(${makeQuantityPyExpression(Quantity(v1, ft))} + ${makeQuantityPyExpression(Quantity(v2, ft))})" 
      case Quantity(QESubtract(d, v1, v2), ft) => 
        s"(${makeQuantityPyExpression(Quantity(v1, ft))} - ${makeQuantityPyExpression(Quantity(v2, ft))})" 
      case Quantity(OMS(path), ft) => {
          parameters ::= path.name.toString
          ensureIdentifierString(path.name.toString) 
      }
      case Quantity(OMA(OMID(path), x), ft) =>
        throw new scala.MatchError("Undefined operation: " + path.toString())
      case _ => throw new scala.MatchError("Match Error hodgepodge", q)
    }
    
  }
  
  private[this] def makeRulePyLambda(rule: Rule): (String, String) = {
    val expr = new QuantityPyExpression(rule.solution)
    (rule.solved.name.toString(), 
        s"lambda (${expr.parameters.tail.foldRight(expr.parameters.head.toString()){(x, y) => x + "," + y}}): ${expr.expression}")
        
  }
  
  private[this] def quantityDeclsPyAttributes(mpd: MPD) = 
    mpd.quantityDecls.map(q => {Map(
        "name" -> s"'${q.name.toString}'",
        "path" -> s"'${q.path.toString}'",
        "dimension" -> (q.dim match {
          case OMS(p) => s"'${p.name.toString}'"
          case _ => "''"
          })
        )
    })
    
  private[this] def lawsPyAttributes(mpd: MPD) = 
    mpd.laws.map(l => {Map(
        "name" -> s"'${l.name.toString}'",
        "path" -> s"'${l.path.toString}'") ++ 
        l.rules.map(makeRulePyLambda(_))
    })
  
  def pyObjectAssignment(lhs: String, objname: String, attrs: Map[String, String], currentIndentLevel: Int): String = {
    s"""$lhs = type('$objname', (object,) , {
${pyIndent(currentIndentLevel+1)}${attrs.map(kp => s"'${kp._1}': ${kp._2}").mkString(",\n"+pyIndent(currentIndentLevel+1))} 
${pyIndent(currentIndentLevel+1)}})()"""
  }
    
  def constructPyFromMPD(mpdOpt: Option[MPD]) : Option[String] = {
    val mpd = mpdOpt.getOrElse(return None)
    val py = s"""
class MPD_${mpd.name.toString()}
${pyIndent(1)}laws = {}
${pyIndent(1)}quantity_decls = {}

${pyIndent(1)}def __init__(self):
${pyIndent(2)}self.init_quantity_decls()
${pyIndent(2)}self.init_laws()

${pyIndent(1)}def __getitem__(self, quantity_decl_key):
${pyIndent(2)}return self.quantity_decls[quantity_decl_key]

${pyIndent(1)}def __call__(self, law_key):
${pyIndent(2)}return self.laws[law_key]

${pyIndent(1)}def init_quantity_decls(self):
${pyIndent(2)}${quantityDeclsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.quantity_decls[${mp("name")}]", "QuantityDecl", mp, 2)}.mkString("\n\n"+pyIndent(2))
}

${pyIndent(1)}def init_laws(self):
${pyIndent(2)}${lawsPyAttributes(mpd).map{
  mp => pyObjectAssignment(s"self.laws[${mp("name")}]", "Law", mp, 2)}.mkString("\n\n"+pyIndent(2))
}
"""
    Some(py)
  }
  
  def exportTheory(thy: DeclaredTheory, bf: BuildTask) {
    println(thy.name)
    val pyOpt = thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Models?MPD" => constructPyFromMPD(mpdtool.toMPD(thy))
       case _ => return
    }
    
    pyOpt.foreach(py => {
      println(py)

      val header = "#!/usr/bin/env python\n\n# "+thy.path.toString()+"\n\n"
      utils.File.write(bf.outFile, header+py)
    })
  }

  def exportNamespace(dpath: DPath, bd: BuildTask, namespaces: List[BuildTask], modules: List[BuildTask]) {}
  
  def exportDocument(doc: Document, bf: BuildTask) {}
  
  def exportView(view: DeclaredView, bf: BuildTask) {}
}

/*
 * 
 */
