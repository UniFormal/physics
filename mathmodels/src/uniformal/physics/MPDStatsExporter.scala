package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._
import utils._
import objects._

class MPDStatsExporter extends Exporter { 
   val key = "mpd-stats"

   private lazy val mpdtool = {
    val m = new MPDTool(controller)
    m
   }
   
   def exportTheory(thy: DeclaredTheory, bf: BuildTask) { 
    val statsOpt = thy.meta match {
       case Some(x) if x.toString() == "http://mathhub.info/MitM/Foundation/Units?ModelBase" => {
         mpdtool.toMPD(thy).get.prettyListCycles             
       }
       case _ => return
    }
    var output = ""    
    statsOpt.foreach(f => {
      output += f.map((t) => s"${t._1} => ${t._2}").mkString(">", "->", ">") + "\n"
    })
    utils.File.write(bf.outFile, output)
  }
   
  def exportNamespace(dpath: DPath, bd: BuildTask, namespaces: List[BuildTask], modules: List[BuildTask]) {}
  
  def exportDocument(doc: Document, bf: BuildTask) {}
  
  def exportView(view: DeclaredView, bf: BuildTask) {}
}