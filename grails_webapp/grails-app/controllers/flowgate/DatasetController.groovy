package flowgate

class DatasetController {

    def axSelAllCandFcs(Experiment experiment) {
        def expFileCandidatesList = getFilteredList(experiment)
        Dataset ds = Dataset.get(params?.dsId?.toLong())
        session.fCandSels = expFileCandidatesList*.id
        render(contentType: 'text/json') {
            success true
            fcsCandList "${g.render(template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds: ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList])}"
        }
    }

    def axDeselAllCandFcs(Experiment experiment) {
        def expFileCandidatesList = getFilteredList(experiment)
        Dataset ds = Dataset.get(params?.dsId?.toLong())
        session.fCandSels = []
        render(contentType: 'text/json') {
            success true
            fcsCandList "${g.render(template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds: ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList])}"
        }
    }

    def axSelAllAssFcs(Dataset ds) {
        def expFileAssignedList = ds.expFiles
        session.fAssSels = expFileAssignedList*.id
        render(contentType: 'text/json') {
            success true
            fcsAssList "${g.render(template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds: ds, dsId: ds.id, expFileAssignedList: expFileAssignedList])}"
        }
    }

    def axDeselAllAssFcs(Dataset ds) {
        def expFileAssignedList = ds.expFiles
        session.fAssSels = []
        render(contentType: 'text/json') {
            success true
            fcsAssList "${g.render(template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds: ds, dsId: ds.id, expFileAssignedList: expFileAssignedList])}"
        }
    }

    def axSetFilter(Experiment experiment) {
        String mdQval = params.eMetaVal - 'mValCb_'
        if (params.ckStatus == "true") {
            if (session.filters) {
                if (!session.filters.flatten().contains(mdQval))
                    session.filters.add(mdQval)
            } else
                session.filters = [mdQval]
        } else {
            if (session.filters)
                session.filters.remove(mdQval)
            else
                session.filters = [mdQval]
        }
        def expFileCandidatesList = getFilteredList(experiment)
        Dataset ds = Dataset.get(params?.dsId?.toLong())
        render(contentType: 'text/json') {
            success true
            fcsList "${g.render(template: 'datasetTmpl/fcsFileCandidates', model: [experiment: experiment, ds: ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList])}"
        }
    }

    def getFilteredList(Experiment experiment) {
        def expFileCandidatesList
        if (session.filters) {
            session.filters.each { filterVal ->
                if (expFileCandidatesList)
                    expFileCandidatesList += ExpFileMetadata.findAll { (mdVal == filterVal) && (expFile.experiment == experiment) }*.expFile
                else
                    expFileCandidatesList = ExpFileMetadata.findAll { (mdVal == filterVal) && (expFile.experiment == experiment) }*.expFile
            }
        } else {
            expFileCandidatesList = ExpFileMetadata.findAll { expFile.experiment == experiment }*.expFile
            //TODO check this
            //  if there is no annotation done??? expect empty list here, fill with experiment expfiles
            if (expFileCandidatesList.size() < 1) {
                expFileCandidatesList = experiment?.expFiles
            }
        }
        return expFileCandidatesList?.unique()
    }

    def axDsChange(Dataset dataset) {
        session.fAssSels = []
        renderPanels(dataset)
    }

    def renderPanels(ds) {
        def expFileList = ds.expFiles
        println "render panels ds:${ds.name} files: ${expFileList}"
        render(contentType: 'text/json') {
            success true
            metaData "${g.render(template: 'datasetTmpl/mdFilterPanel', model: [experiment: ds.experiment, ds: ds, dsId: ds.id, expFileList: expFileList])}"
//      fcsCandidates "${g.render (template: 'datasetTmpl/fcsFileCandidates', model: [experiment: ds.experiment, ds:ds, dsId: ds.id, expFileList: expFileList ]) }"
            dsPanel "${g.render(template: 'datasetTmpl/datasetPanel', model: [experiment: ds.experiment, ds: ds, dsId: ds.id])}"
            fcsAssigned "${g.render(template: 'datasetTmpl/fcsFileAssigned', model: [experiment: ds.experiment, ds: ds, dsId: ds.id, expFileAssignedList: expFileList])}"
        }
    }

    def axAddDs(Experiment experiment) {
        Dataset ds = new Dataset()
        params.dsMode = 'dsCreate'
        render(contentType: 'text/json') {
            success true
            dsField "${g.render(template: 'datasetTmpl/datasetNameCreate', model: [experiment: experiment, ds: ds, dsMode: 'dsCreate'])}"
            fcsAssList "${g.render(template: 'datasetTmpl/fcsFileAssigned', model: [experiment: experiment, ds: ds, dsId: ds.id, expFileAssignedList: []])}"
        }
    }

    def axEditDs(Dataset dataset) {
        params.dsMode = 'dsEdit'
        println "in axEditDs"
        render(contentType: 'text/json') {
            success true
            dsField "${g.render(template: 'datasetTmpl/datasetNameEdit', model: [experiment: dataset.experiment, dsId: dataset.id, ds: dataset, dsMode: 'dsEdit'])}"
        }
    }

    def index() {
        Experiment experiment = Experiment.findById(params?.eId)
        def datasetList = Dataset.findAllByExperiment(experiment)
        respond datasetList, model: [datasetCount: datasetList.size(), eId: params?.eId, experiment: experiment]
    }

    def create() {
        //TO DO check if dataseet with same name exist or not
        Dataset dataset = new Dataset(params)
        Experiment experiment = Experiment.findById(params.eId)
        def expFileCandidatesList = getFilteredList(experiment)
        respond dataset, model: [eId: params.eId, experiment: experiment, expFileCandidatesList: expFileCandidatesList, name: params.name, description: params.description]
    }

    def edit(Dataset ds) {
        if (ds != null) {
            Experiment experiment = ds.experiment
            def expFileCandidatesList = getFilteredList(experiment)
            render view: 'edit', model: [experiment: experiment, dsId: ds.id, dataset: ds, expFileCandidatesList: expFileCandidatesList], params: params
        } else {
            redirect controller: 'project',  action: 'list', params: [eId: params?.eId]
        }
    }

    def ds_edit(Experiment experiment) {
        Dataset ds = params.dsId ? Dataset.get(params.dsId) :
            Dataset.findAllByExperiment(experiment) ?
                Dataset.findAllByExperiment(experiment)?.first() :
                    new Dataset(experiment:experiment, name: 'new dataset', expFiles: [], description: '[]').save(flush: true)
        def expFileCandidatesList = getFilteredList(experiment)
        render view: 'ds_edit', model: [experiment: experiment, dsId: ds?.id, ds: ds, expFileCandidatesList: expFileCandidatesList], params: params
    }

    def assign() {
      Experiment experiment = params?.expId ? Experiment.get(params?.expId?.toLong()) : params.eId ? Experiment.get(params?.eId?.toLong()) : null
      Dataset ds
      if(params?.dsMode == 'dsCreate') {
        params.experiment = experiment
        ds = new Dataset(params)
      }
      else {
        if(params?.dsId){
          ds = Dataset.get(params?.dsId.toLong())
        }
      }

      ds.name = params?.name ?: ds.name
      session.fCandSels = []
      session.fAssSels = []

      (params.findAll { key, value -> key.startsWith('cbFcsFileCandId_') }).each {
        if (it.value == 'on') {
          def fcsId = (it?.key - 'cbFcsFileCandId_').toLong()
            ExpFile candidate = ExpFile.get(fcsId)
            def debugdummy = ds.expFiles.findAll { it.id == candidate.id }
            if (!(ds?.expFiles.findAll { it.id == candidate.id })) {
              println "expFiles size ${ds?.expFiles?.size()}"
              if (ds?.expFiles) {
                ds?.expFiles?.add(candidate)
              }
              else {
                ds.expFiles = [candidate]
              }
            }
            else {
              println "already in list, no need to add"
            }
            if (session.fAssSels)
              session.fAssSels.add(fcsId)
            else
              session.fAssSels = [fcsId]
            flash.message = "FCS files are successfully assigned"
          }
      }
      ds.save(flush: true)
      params.dsId = ds.id
      redirect action: 'ds_edit', model: [dsId: ds.id, dsMode: "dsEdit"], params: [dsId: ds.id], id: params?.expId ? params?.expId : params?.eId ? params?.eId : null
    }

    def remove() {
        Experiment experiment = params?.expId ? Experiment.get(params?.expId?.toLong()) : params.eId ? Experiment.get(params?.eId?.toLong()) : null
        Dataset ds
        if(params?.dsMode == 'dsCreate') {
            params.experiment = experiment
            ds = new Dataset(params)
        }
        else {
            if(params?.dsId){
                ds = Dataset.get(params?.dsId.toLong())
            }
        }
        ds.name = params?.name ?: ds.name
        session.fAssSels = []
        session.fCandSels = []

        (params.findAll { key, value -> key.startsWith('cbFcsFileAssId_') }).each {
            if (it.value == 'on') {
                def fcsId = (it?.key - 'cbFcsFileAssId_').toLong()
                ds.expFiles.remove(ExpFile.get(fcsId))
                println "removed fcsFileId ${fcsId}"
                if (session.fCandSels)
                    session.fCandSels.add(fcsId)
                else
                    session.fCandSels = [fcsId]
            }
        }
        ds.save(flush: true)

        flash.message = "FCS files are successfully removed"
//        redirect action: 'edit', id: ds.id
//        redirect action: 'ds_edit', id: experiment?.id, params: [dsId: ds.id]
        redirect action: 'ds_edit', model: [dsId: ds.id, dsMode: "dsEdit"], params: [dsId: ds.id], id: params?.expId ? params?.expId : params?.eId ? params?.eId : null
    }


    def save(params) {
        Experiment experiment = Experiment.findById(params.eId)
        Dataset exist = Dataset.findByNameAndExperiment(params.name, experiment)
        if(exist != null) {
            flash.error = "Dataset with " + params.name + " is already exist in this experiment"
        } else if(params.name == null || params.name.toString().equals("")) {
            flash.error = "Name is required!"
        } else {
            Dataset ds = new Dataset(experiment: experiment, name: params.name, description: params.description, expFiles: [])
            (params.findAll { key, value -> key.startsWith('file_') }).each {
                if (it.value == 'on') {
                    def fcsId = (it?.key - 'file_').toLong()
                    ExpFile candidate = ExpFile.get(fcsId)
                    ds.expFiles.add(candidate)
                }
            }
            ds.save(flush: true)
            flash.message = "New dataset is successfully created"
            redirect action: 'index', params: [eId: params?.eId]
            return
        }

        (params.findAll { key, value -> key.startsWith('file_') }).each {
            if (it.value == 'on') {
                def fcsId = (it?.key - 'file_').toLong()
                if (session.selectedFiles)
                    session.selectedFiles.add(fcsId)
                else
                    session.selectedFiles = [fcsId]
            }
        }

        redirect action: 'create', params: params
        return
    }

    def update(params) {
        Experiment experiment = Experiment.findById(params.eId)
        Dataset exist = Dataset.findByNameAndExperiment(params.name, experiment)
        if(exist == null || exist.id == params.id.toLong()) {
            Dataset ds = Dataset.get(params.id.toLong())
            ds.name = params.name
            ds.description = params.description
            ds.expFiles = []

            (params.findAll { key, value -> key.startsWith('file_') }).each {
                if (it.value == 'on') {
                    def fcsId = (it?.key - 'file_').toLong()
                    ds.expFiles.add(ExpFile.get(fcsId))
                }
            }

            ds.save(flush: true)
            flash.message = "Dataset is successfully updated"
        } else {
            flash.message = "Dataset with " + params.name + " is already exist in this experiment"
        }
        redirect action: 'edit', id: params.id
    }

    def dsExit() {
        session.fAssSels = []
        session.fCandSels = []
        println "dataset submit (action update)"
        redirect controller: 'experiment', action: 'index', params: [eId: params.expId] //, params: params
    }

}
