package flowgate

class DatasetController {

  def axSelAllCandFcs(Experiment experiment){
    def expFileCandidatesList = getFilteredList(experiment)
    Dataset ds = Dataset.get(params?.dsId?.toLong())
    session.fCandSels = expFileCandidatesList*.id
    render(contentType: 'text/json') {
      success true
      fcsCandList "${g.render (template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds:ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList ]) }"
    }
  }

  def axDeselAllCandFcs(Experiment experiment){
    def expFileCandidatesList = getFilteredList(experiment)
    Dataset ds = Dataset.get(params?.dsId?.toLong())
    session.fCandSels = []
    render(contentType: 'text/json') {
      success true
      fcsCandList "${g.render (template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds:ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList ]) }"
    }
  }

 def axSelAllAssFcs(Dataset ds){
    def expFileAssignedList = ds.expFiles
    session.fAssSels = expFileAssignedList*.id
    render(contentType: 'text/json') {
      success true
      fcsAssList "${g.render (template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileAssignedList: expFileAssignedList ]) }"
    }
  }

  def axDeselAllAssFcs(Dataset ds){
    def expFileAssignedList = ds.expFiles
    session.fAssSels = []
    render(contentType: 'text/json') {
      success true
      fcsAssList "${g.render (template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileAssignedList: expFileAssignedList ]) }"
    }
  }

  def axSetFilter(Experiment experiment){
    String mdQval = params.eMetaVal-'mValCb_'
    if(params.ckStatus=="true"){
      if(session.filters){
        if(!session.filters.flatten().contains(mdQval))
          session.filters.add(mdQval)
      }
      else
        session.filters = [mdQval]
    }
    else{
      if(session.filters)
        session.filters.remove(mdQval)
      else
        session.filters = [mdQval]
    }
    def expFileCandidatesList = getFilteredList(experiment)
    Dataset ds = Dataset.get(params?.dsId?.toLong())
    render(contentType: 'text/json') {
      success true
      fcsList "${g.render (template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds:ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList ]) }"
    }
  }

  def getFilteredList(Experiment experiment){
    def expFileCandidatesList
    if(session.filters){
      session.filters.each{ filterVal ->
        if(expFileCandidatesList)
          expFileCandidatesList += ExpFileMetadata.findAll{(mdVal == filterVal) && (expFile.experiment == experiment)}*.expFile
        else
          expFileCandidatesList = ExpFileMetadata.findAll{(mdVal == filterVal) && (expFile.experiment == experiment)}*.expFile
      }
    }
    else{
      expFileCandidatesList = ExpFileMetadata.findAll{expFile.experiment == experiment}*.expFile
      //TODO check this
      //  if there is no annotation done??? expect empty list here, fill with experiment expfiles
      if(expFileCandidatesList.size()<1){
        expFileCandidatesList = experiment.expFiles
      }
    }
    return expFileCandidatesList.unique()
  }

  def axDsChange(Dataset dataset){
    session.fAssSels = []
    renderPanels(dataset)
  }

  def renderPanels(ds){
    def expFileList = ds.expFiles
    println "render panels ds:${ds.name}"
    render (contentType: 'text/json'){
      success true
      metaData "${g.render (template: 'datasetTmpl/mdFilterPanel', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileList: expFileList ]) }"
//      fcsCandidates "${g.render (template: 'datasetTmpl/fcsFileCandidates', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileList: expFileList ]) }"
      dsPanel "${g.render (template: 'datasetTmpl/datasetPanel', model: [experiment: ds.experiment, ds:ds, dsId: ds.id ]) }"
      fcsAssigned "${g.render (template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileAssignedList: expFileList ]) }"
    }
  }

  def axAddDs(Experiment experiment){
    Dataset ds = new Dataset()
    params.dsMode = 'dsCreate'
    render(contentType: 'text/json') {
      success true
      dsField "${g.render(template: 'datasetTmpl/datasetFieldCreate', model: [dataset: ds, dsMode:'dsCreate'])}"
      fcsAssList "${g.render (template: 'datasetTmpl/fcsFileAssigned', model: [experiment: experiment, ds:ds, dsId: ds.id, expFileAssignedList: [] ]) }"
    }
  }

  def axEditDs(Dataset dataset){
    params.dsMode = 'dsEdit'
    render(contentType: 'text/json') {
      success true
      dsField "${g.render(template: 'datasetTmpl/datasetFieldCreate', model: [dataset: dataset, dsMode:'dsEdit'])}"
    }
  }


  def index() {}



  def ds_edit(Experiment experiment) {
    Dataset ds = params.dsId ? Dataset.get(params.dsId) :
                                 Dataset.findAllByExperiment(experiment) ?
                                   Dataset.findAllByExperiment(experiment)?.first() :
                                     new Dataset(experiment:experiment, name: 'new dataset', expFiles: [], description: '[]').save(flush: true)
    def expFileCandidatesList = getFilteredList(experiment)
    render view: 'ds_edit', model: [experiment: experiment, dsId: ds.id, ds: ds, expFileCandidatesList: expFileCandidatesList], params: params
  }

  def assign(){
    Experiment experiment = Experiment.get(params?.expId)
    if(params.formMode == 'dsCreate'){
      dsSave(experiment, params)
      return
    }
    if(params.formMode == 'dsEdit'){
      dsUpdate(experiment, params)
      return
    }
    Dataset ds = Dataset.get(params?.ds?.id?.toLong())
    session.fCandSels = []
    session.fAssSels = []

    (params.findAll { key, value -> key.startsWith('cbFcsFileCandId_')}).each{
      if(it.value == 'on'){
        def fcsId = (it?.key - 'cbFcsFileCandId_').toLong()
        ExpFile candidate = ExpFile.get(fcsId)
        def debugdummy = ds.expFiles.findAll{it.id==candidate.id}
        if(!(ds.expFiles.findAll{it.id==candidate.id})){
          if(ds.expFiles ){
            ds.expFiles.add(candidate)
          }
          else{
            ds.expFiles = [candidate]
          }
        }
        else{
          println "already in list, no need to add"
        }
        if(session.fAssSels)
          session.fAssSels.add(fcsId)
        else
          session.fAssSels = [fcsId]
      }
    }
    ds.save(flush: true)

    redirect action: 'ds_edit', id: experiment.id, params:[dsId: ds.id]
  }

  def remove(){
    Experiment experiment = Experiment.get(params?.expId)
    Dataset ds = Dataset.get(params?.ds?.id?.toLong())
    session.fAssSels = []
    session.fCandSels = []

    (params.findAll { key, value -> key.startsWith('cbFcsFileAssId_')}).each{
      if(it.value == 'on'){
        def fcsId = (it?.key - 'cbFcsFileAssId_').toLong()
        ds.expFiles.remove(ExpFile.get(fcsId))
        println "removed fcsFileId ${fcsId}"
        if(session.fCandSels)
          session.fCandSels.add(fcsId)
        else
          session.fCandSels = [fcsId]
      }
    }
    ds.save(flush: true)

    redirect action: 'ds_edit', id: experiment.id, params:[dsId: ds.id]
  }


  def dsSave(Experiment experiment, params){
    Dataset ds = new Dataset(experiment: experiment, name: params.dataset.name, description: '[]', expFiles: [] )
    ds.save()
    redirect action: 'ds_edit', id: experiment.id, params: [dsId: ds.id ]
  }

  def dsUpdate(Experiment experiment, params) {
    Dataset ds = Dataset.get(params.dsId.toLong())
    ds.name = params.dataset.name
    ds.save( flush: true)
    redirect action: 'ds_edit', id: experiment.id, params: [dsId: ds.id ]
  }

  def dsExit(){
    session.fAssSels = []
    session.fCandSels = []
    println "dataset submit (action dsUpdate)"
    redirect controller: 'experiment', action: 'index', params:[eId: params.expId] //, params: params
  }

}
