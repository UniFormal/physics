package uniformal.physics

import info.kwarc.mmt.api._
import documents._
import modules._
import symbols._
import archives._

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
  
  def exportTheory(thy: DeclaredTheory, bf: BuildTask) {
     val mpd = mpdtool.toMPD(thy)
     
     //val py = mpd.toString
     
     //utils.File.write(bf.outFile, py)
  }

  def exportNamespace(dpath: DPath, bd: BuildTask, namespaces: List[BuildTask], modules: List[BuildTask]) {}
  def exportDocument(doc: Document, bf: BuildTask) {}
  def exportView(view: DeclaredView, bf: BuildTask) {}
}

/*
 * 
 */
