package flowgate

import grails.core.GrailsApplication
import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(['ROLE_Admin','ROLE_User'])
@Transactional(readOnly = true)
class ExpFileController {

    def fcsService
    def chkSumService
    def utilsService
    def springSecurityService

    GrailsApplication grailsApplication

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
            transactionStatus.setRollbackOnly()
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
            transactionStatus.setRollbackOnly()
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
    }

    def expFileCreate2(){
    }

    def uploadFcsFiles(){
//        TODO remove hardcoded experiment and change to params!! for developing shortcut only!!!
        Experiment experiment = Experiment.get(1)
//        def partFiles = request.getFiles("actFcsFile")
//        partFiles.each{ file ->
//            println file.getOriginalFilename()
//
//        }
        String fcsStoragePath = grailsApplication.config.getProperty('fcsFileStoreLocation.path', String)
        def ulFiles = request.multiFileMap.get("actFcsFile")
        if(ulFiles.findAll { !it.empty }.size<1){
            flash.errMsg = 'E: no file selected!'
            redirect action: 'expFileCreate', eId: experiment.id
            return

        }
        else {
            ulFiles.each { fcsFile ->
                println fcsFile.originalFilename
                if (fcsFile.part.fileItem.tempFile.exists()) {
                    def sha1 = chkSumService.getSha1sum(fcsFile.part.fileItem.tempFile)
                    def md5 = chkSumService.getMD5sum(fcsFile.part.fileItem.tempFile)
                    if(ExpFile.findAllByChkSum(sha1)){
                        println "file with same checksum (${sha1}) already exists"
                    }
                    fcsService.readFile(fcsFile.part.fileItem.tempFile, false)
                    ExpFile expFile = new ExpFile(experiment: experiment, chkSum: sha1, title: fcsFile.originalFilename, fileName: fcsFile.originalFilename, filePath: fcsStoragePath, createdBy: springSecurityService.currentUser).save()
                    fcsFile.transferTo(new File("${fcsStoragePath}${fcsFile.originalFilename}"))
                    experiment.expFiles.add(expFile)
                }
            }
            redirect action: 'annotationTbl', id: experiment.id   //, model: [expi: Experiment.get(1), expId2: Experiment.get(1)]
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

    def axMetaChange(ExperimentMetadata eMeta){
        ExperimentMetadataValue eMetaVal = ExperimentMetadataValue.get(params.valId.toLong())
        session["meta_${eMeta.mdKey}"] = eMetaVal.mdValue
        Experiment experiment = eMeta.experiment
        render (contentType:"text/json") {
            success true
            tablTabl "${g.render(template: 'annotationTmpl/tablTmpl', model: [experiment: experiment])}"
        }
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
