package flowgate

import grails.core.GrailsApplication
import grails.plugin.springsecurity.annotation.Secured
import grails.util.Environment

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(['ROLE_Admin','ROLE_User'])
//@Transactional(readOnly = true)
class ExpFileController {

    def fcsService
    def chkSumService
    def utilsService
    def springSecurityService

    GrailsApplication grailsApplication

    def axMetaValAdd(){
         /*
        contentPage "${g.render( template: 'expFileTmpl', model: [experiment: experiment, expFile: expFile])}"
        */
        println "in Controller expFile action axMetaValAdd"
        ExperimentMetadataValue expMetaDatVal = new ExperimentMetadataValue()
        render (contentType:"text/json"){
            success true
            expMetaDatOldRow "${g.render( template: '/expFile/annotationTmpl/expMetaDatValOldRow', model: [expMetaDatVal: expMetaDatVal])}"
            expMetaDatNewRow "${g.render( template: '/expFile/annotationTmpl/expMetaDatValNewRow', model: [expMetaDatVal: expMetaDatVal])}"
        }
    }

    def axExpFileToggleClick() {
        Experiment experiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        ExpFile expFile = ExpFile.findByIdAndIsActive(params?.expFileId.toLong(), true)
        if(experiment && expFile){
            if(session.expFileOpenIds?.contains(expFile.id)){
                session.expFileOpenIds?.remove(session.expFileOpenIds?.indexOf(expFile.id))
            }
            else {
                session.expFileOpenIds?.push(expFile.id)
            }
//            expFile.save(failOnSafe: true, flush: true)
//            TODO catch the save error
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            render (contentType:"text/json"){
                success true
                contentTree "${g.render( template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render( template: 'expFileTmpl', model: [experiment: experiment, expFile: expFile])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success false
                msg "${g.message(message: 'error.experiment.noActive.message')}"
            }
        }
    }

    //    TODO do collapseAll and expandAll contolled by a param in one method
    def collapseAll(){
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        if(experiment){
            ExpFile.findAllByExperimentAndIsActive(experiment, true).each {
                if(session?.expFileOpenIds?.contains(it.id)) session?.expFileOpenIds?.remove(session?.expFileOpenIds?.indexOf(it.id))
            }
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            render (contentType:"text/json"){
                success true
                contentTree "${g.render( template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: experiment])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success false
                msg "${g.message(message: 'error.experiment.notGot.message')}"
            }
        }
    }

    //    TODO do collapseAll and expandAll contolled by a param in one method
    def expandAll(){
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        if(experiment){
            ExpFile.findAllByExperimentAndIsActive(experiment, true).each {
                if(!session.expFileOpenIds)
                {
                    session.expFileOpenIds = []
                }
                if(!session?.expFileOpenIds?.contains(it.id)) session?.expFileOpenIds?.push(it.id)
            }
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            render (contentType:"text/json"){
                success true
                contentTree "${g.render( template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: experiment])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success false
                msg "${g.message(message: 'error.experiment.notGot.message')}"
            }
        }
    }

    def axMetaChange(ExperimentMetadata eMeta){
        ExperimentMetadataValue eMetaVal = ExperimentMetadataValue.get(params.valId.toLong())
        session["meta_${eMeta.mdKey}"] = eMetaVal.mdValue
        Experiment experiment = eMeta.experiment
//        TODO check values; index data
        render (contentType:"text/json") {
            success true
            tablTabl "${g.render(template: 'annotationTmpl/tablTmpl', model: [experiment: experiment])}"
        }
    }

    def axMetaActionChange(ExperimentMetadata eMeta){
      println "axMetaActionChange ${params.colAction}  emetaData=${eMeta.dump()}!"
      switch (params.colAction){
        case 'Delete': delCol(eMeta)
                       return
          break

        case 'Hide': hideCol(eMeta)
                     return
          break

        case 'Edit': Experiment experiment = eMeta.experiment
                     println "emeta = ${eMeta.dump()}"
                     render (contentType:"text/json") {
                       success true
                       edModalTmpl "${g.render(template: 'annotationTmpl/colEditModal', model: [experiment: experiment, eMeta: eMeta, category: eMeta.mdCategory])}"
                     }
          break

        default:  Experiment experiment = eMeta.experiment
                  render (contentType:"text/json") {
                    success true
                    tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: eMeta.mdCategory])}"
                  }
      }
    }

    def axShowAllCols(Experiment experiment){
      println "showCols "
      experiment.expMetadatas.each{
        setColVisability(it, true)
      }
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: params.category])}"
      }
    }

    def hideCol(ExperimentMetadata eMeta){
      println "hide ${eMeta.mdKey}"
      Experiment experiment = eMeta.experiment
      String category = eMeta.mdCategory
      setColVisability(eMeta, false)
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: category])}"
      }
    }

    def setColVisability(ExperimentMetadata eMeta, Boolean visible){
      eMeta.visible = visible
      eMeta.save flush: true
    }



    def delCol(ExperimentMetadata eMeta){
      println "delete eMeta ${eMeta.mdKey}"
      Experiment experiment = eMeta.experiment
      String category = eMeta.mdCategory
      eMeta.mdVals*.delete()
      eMeta.mdVals.clear()
      eMeta.delete flush: true
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: category])}"
      }
    }

    def addColumn(Experiment experiment){ //add metaData column in annotation table
        params.experiment = experiment
        ExperimentMetadata eMetaData = new ExperimentMetadata(params)
        eMetaData.save(flush: true)
        params.expMetaData = eMetaData
        ExperimentMetadataValue eMetaValue = new ExperimentMetadataValue(params)
        eMetaValue.save(flush: true)
        if(eMetaData?.mdVals){
          eMetaData?.mdVals?.add(eMetaValue)
        }
        else{
          eMetaData?.mdVals = [eMetaValue]
        }
        println 'add metaData column'
        redirect action: 'annotationTbl', id: experiment.id
    }

    def axAddColumn(Experiment experiment){
        println " axAddColumn exp=${experiment?.id}"
        render (contentType:"text/json") {
            success true
            crModalTmpl "${g.render (template: 'annotationTmpl/colCreateModal', model: [experiment: experiment, category: params?.category])}"
        }
    }

    def editColumn(Experiment experiment){ //add metaData column in annotation table
        ExperimentMetadata eMetaData = ExperimentMetadata.get(params?.metaId?.toLong())
        eMetaData.save(flush: true)
        ExperimentMetadataValue eMetaValue = ExperimentMetadataValue(params?.metaValId?.toLong())
        eMetaValue.save(flush: true)
        eMetaData.mdVals.add(eMetaValue)
        println 'add metaData column'
        redirect action: 'annotationTbl', id: experiment.id
    }

    def axExpFileCreate(){
        ExpFile expFile = new ExpFile()
        expFile.experiment = Experiment.get(params?.eId?.toLong())
        expFile.isActive = true
//        TODO get unique name
        expFile.title = 'new exp file'
        Boolean saveOk = expFile.save flush:true
        if(saveOk) {
            session.expFileOpenIds?.push(expFile.id)
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            Project project = expFile.experiment.project
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
            flash.message = 'exp file successfully added'
            render (contentType:"text/json") {
                success true
                contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: expFile.experiment])}"
            }
        }
        else {
            flash.message = 'something wrong with creating new exp file! please check the logs'
            render (contentType:"text/json") {
//                TODO check what templates to render in case of error
                success false
//                contentBtnBar flash.message
//                contentTree flash.message
//                contentPage flash.message
            }
        }
    }

    @Transactional
    def axDeleteExpFile() {
        ExpFile expFile = ExpFile.get(params?.expFileId?.toLong())
        if (expFile == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        expFile.isActive = false
        Boolean saveOk = expFile.save flush:true
        if(saveOk) {
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            Project project = expFile.experiment.project
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'expFile.label', default: 'FcsFileContainer'), expFile.id])
            render (contentType:"text/json") {
                success true
                contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: expFile.experiment])}"
            }
        }
        else {
            flash.message = message(code: 'default.delete.error.message', args: [message(code: 'expFile.label', default: 'Exp. File'), expFile.id], default: 'error deleting container! ')
            render (contentType:"text/json") {
//                TODO check what templates to render in case of error
                success false
//                contentBtnBar flash.message
//                contentTree flash.message
//                contentPage flash.message
            }
        }

//        redirect controller: 'experiment', view: 'index', params: [pId: fcsFileContainer.experiment.project.id, eId: fcsFileContainer.experiment.id]
        request.withFormat {
        form multipartForm {
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'expFile.label', default: 'Exp. File'), expFile.id])
            redirect action:"index", method:"GET"
        }
        '*'{ render status: NO_CONTENT }
        }
    }

    @Transactional
    def erase(ExpFile expFile) {

        if (expFile == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        expFile.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'expFile.label', default: 'Exp. File'), fcsFileContainer.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }

    def expFileCreate(){
        Experiment experiment = Experiment.findByIdAndIsActive(params?.eId?.toLong(), true)
        if(experiment) {
            User user = springSecurityService.currentUser
            ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment?.project, true)
            respond Experiment.get(params?.eId), model: [owner: springSecurityService.currentUser, eId: params?.eId, projectList: projectList, experimentList: experimentList]
        } else {
            if(experiment?.project){
                redirect controller: 'project', view: 'index'
            }
            else {
                request.withFormat {
                    'html' { render view: '/error2', model: [message: g.message(message: 'error.experiment.noActive.message')]  }
                    'json' { render(contentType: 'text/json') { 'this should go to gson view!'  }
                    }
                }
            }
        }
    }

    def expFileCreate2(){
    }


    def importAnnotation(Experiment experiment){
        def annotationCsvFile = request.getFile("annotationFile")
        File annCsvFile = annotationCsvFile.part.fileItem.tempFile
        if(annCsvFile.size()<1){
            println "error: no file selected"
            flash.msg = "Error: No file selected!"
            doAnnotate(experiment)
            return
        }
        def headers = []
        def rows = []
        Integer lineCntr = 0
        annCsvFile.splitEachLine(','){ fields ->
            if(lineCntr == 0){
                headers = fields
            }
            else{
                rows += [fields]
            }
            lineCntr += 1
        }
//        println "headers ${headers}"
//        println "rows ${rows}"
        def myMap = []
        rows.each { row ->
            myMap += [[headers, row].transpose().collectEntries()]
        }
//        println " myMap = ${myMap}"
        utilsService.csvMetadataParse(experiment, myMap, headers)
        doAnnotate(experiment)
        return
    }

    def uploadFcsFiles(){
        try {
            Experiment experiment = Experiment.get(params.eId.toLong())
            String fcsStoragePath = grailsApplication.config.getProperty('fcsFileStoreLocation.path', String)
            def ulFiles = request.multiFileMap.get("actFcsFile")
            if(ulFiles.findAll { !it.empty }.size<1){
                flash.errMsg = 'No file selected!'
                redirect action: 'expFileCreate', eId: experiment.id, params: [eId: experiment.id]
                return
            }
            else {
                ulFiles.each { fcsFile ->
                    println fcsFile.originalFilename
                    if (fcsFile.part.fileItem.tempFile.exists()) {
                        if(ExpFile.findAllByFileNameAndExperiment(fcsFile.originalFilename, experiment))
                            throw new Exception("File with same name already exists")

                        def sha1 = chkSumService.getSha1sum(fcsFile.part.fileItem.tempFile)
                        def md5 = chkSumService.getMD5sum(fcsFile.part.fileItem.tempFile)
                        if(ExpFile.findAllByChkSumAndExperiment(sha1, experiment))
                            throw new Exception("File with same checksum (${sha1}) already exists")

                        fcsService.readFile(fcsFile.part.fileItem.tempFile, false)
                        String filePath = fcsStoragePath + File.separator + experiment.project.id + File.separator + experiment.id + File.separator
                        ExpFile expFile = new ExpFile(experiment: experiment, chkSum: sha1, title: fcsFile.originalFilename, fileName: fcsFile.originalFilename, filePath: filePath, createdBy: springSecurityService.currentUser).save()
                        File file = new File("${filePath}${fcsFile.originalFilename}");
                        file.getParentFile().mkdirs() // create parent folders if not exist
                        fcsFile.transferTo(file)
                        experiment.expFiles.add(expFile)
                    }
                }
                /*if(Environment.current == Environment.DEVELOPMENT) {
                    redirect action: 'annotationTbl', id: experiment.id   //, model: [expi: Experiment.get(1), expId2: Experiment.get(1)]
                }
                else{
                    redirect controller: 'experiment', action: 'index', id: experiment.id, params:[eId: experiment.id]
                }*/

                render (contentType:"text/json") {
                    success true
                }
            }
        } catch (Exception e) {
            render (contentType:"text/json") {
                success false
                msg e.getMessage()
            }
        }
    }

    def toggleFilter(){
        Experiment experiment = Experiment.get(params?.id?.toLong())
        println 'inToggleFilter!'
        if(session.filterFiles) {
            session.filterFiles = !session.filterFiles
            println "session.filterFiles == ${session.filterFiles}"
        }
        else {
            println "init filterFiles = true"
            session.filterFiles = true
        }
        /*render (contentType:"text/json") {
            success true
        }*/
        redirect action: 'annotation', id: experiment.id
    }

    def selAllFcs(Experiment experiment){
        def fSels = experiment.expFiles*.id
//        println "selall experiment ${experiment}; sels ${fSels} "
        session.fSels = fSels
//        redirect action: 'annotation', id: Experiment.get(1).id, sels: sels, model: [sels: sels]
        redirect action: 'annotation', id: experiment.id
    }

    def deselAllFcs(Experiment experiment){
        println "desel experiment ${experiment}"
        session.fSels = []
//        redirect action: 'annotation', id: Experiment.get(1).id, model: [sels: []]
        redirect action: 'annotation', id: experiment.id
    }

    def setCkInitValues(Experiment experiment){
        println "set session init values "
        experiment.expMetadatas.sort{it.mdCategory}?.mdCategory?.unique()?.each{ catgy ->
            experiment.expMetadatas.findAll{it.mdCategory== catgy}.sort{it.dispOrder}.each{ eMeta ->
                if(eMeta.mdVals.size()>1)
                    session["meta_${eMeta.mdKey}"] = eMeta.mdVals.sort{it.dispOrder}.first().mdValue
                else
                    session["meta_${eMeta.mdKey}"] = eMeta.mdVals.mdValue.join(',')
            }
        }
    }

    def axMetaSelect(ExpFile expFile){
        ExperimentMetadata eMeta = ExperimentMetadata.get(params?.mId?.toLong())
        ExperimentMetadataValue eMetaVal = ExperimentMetadataValue.get(params?.metaVal?.toLong())
        if(params.checked=='true'){
            if(!expFile.metaDatas.find{it.mdKey==eMeta.mdKey}){
                ExpFileMetadata efMetaData = new ExpFileMetadata(expFile: expFile, mdKey: eMeta.mdKey, mdVal: eMetaVal.mdValue)
                println "create new ${eMetaVal.mdValue}"
                efMetaData.save(flush: true)
                expFile.metaDatas.add (efMetaData)
            }
            else{
                println "change to ${eMetaVal.mdValue}"
                ExpFileMetadata efMetaData = expFile.metaDatas.find{it.mdKey==eMeta.mdKey}
                efMetaData.mdVal = eMetaVal.mdValue
                efMetaData.save(flush: true)

            }
        }
        if(params.checked=='false'){
            if(!expFile.metaDatas.find{it.mdKey==eMeta.mdKey}){
                println "key does not exist, no need to delete"
            }
            else{
                println "remove ${eMetaVal.mdValue}"
                params.checked = false
                ExpFileMetadata efMetaData = expFile.metaDatas.find{it.mdKey==eMeta.mdKey}
                expFile.metaDatas.remove (efMetaData)
                efMetaData.save( flush: true)
            }
        }

        expFile.save( flush: true)

        render (contentType:"text/json") {
            success true
            cellTabl "${g.render(template: 'annotationTmpl/tablCellTmpl', model: [eMeta: eMeta, expFile: expFile, checked: params.checked ])}"
        }
    }


    def annotation(Experiment experiment){
        render view: 'annotation', model: [experiment: experiment]
    }

    def doAnnotate(Experiment experiment){
//        TODO fill in some logic
        doAnnotation(experiment, params)
        println "session fSels = ${session.fSels}"
//        redirect action: 'annotation', id: experiment.id
        redirect action: 'annotationTbl', id: experiment.id
    }

    def doAnnotation(experiment, params){
        def myParams = [:]
        myParams << params
        (params.findAll { key, value -> key.startsWith('fcsFileId_')}).each{ fcsFileParam ->
            println "fcsFileParam.value = ${fcsFileParam}"
            if(fcsFileParam?.value == 'on'){
                Long fcsId = (fcsFileParam?.key - 'fcsFileId_' - '_chb').toLong()
                utilsService.addIdToSession(fcsId, 'fSels')
                ExpFile fcsFile = ExpFile.get(fcsId)
                (params.findAll { key, value -> key.startsWith('eMeta_')}).each { eMetaParam ->
                    println "metaP - ${eMetaParam}"
                    if(eMetaParam?.value =='on'){
                        Long expMetaId = (eMetaParam?.key - 'eMeta_').toLong()
                        utilsService.addIdToSession(expMetaId, 'anSels')
                        if(!fcsFile.expMetaData.contains(expMetaId)){
                            println "didn't contain, added "
                            ExperimentMetadata eMetaData = ExperimentMetadata.get(expMetaId)
//                            fcsFile.expMetaData.add( ExperimentMetadata.get(expMetaId) )
                            fcsFile.expMetaData.add( eMetaData )
                            String valStr = "eMeta_${expMetaId}.mdVal"
                            def cbVal = myParams.find{ key, value -> key == valStr}.value
                            if(!fcsFile.metaDatas.find{it.mdKey==eMetaData.mdKey}){
                                ExpFileMetadata efMetaData = new ExpFileMetadata(expFile: fcsFile, mdKey: eMetaData.mdKey, mdVal: cbVal)
                                efMetaData.save()
                                fcsFile.metaDatas.add(efMetaData)
                            }
                            else{
                                println "key already exist, new value"

                            }
                        }
                        else{
                            println "allready added"
                        }
                        println "add ${ExperimentMetadata.get(expMetaId).mdVals}"
                    }
                }
                fcsFile.save() //flush: true
//              changes = true
            }
        }
    }

    def doneAnnotation(Experiment experiment){
        println "done annotation! "
        redirect controller: 'experiment', action: 'index', id: experiment.id, params: [eId: experiment.id]
    }

    def annotationTbl(Experiment experiment){
        setCkInitValues(experiment)
        respond experiment
    }

    def doTableAnnotation(Experiment experiment){
        println "in doTableAnnotation"
        render template: 'annMasterTbl', [experiment: experiment]
    }

    def attributeCheck(Experiment experiment){
        println "in attribubeCheck"
        render template: 'annMasterTbl', [experiment: experiment]
    }

}
