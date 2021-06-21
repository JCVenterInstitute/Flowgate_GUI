package flowgate

import grails.converters.JSON
import grails.core.GrailsApplication
import grails.plugin.springsecurity.annotation.Secured
import groovy.json.JsonBuilder
import groovy.json.JsonSlurper
import org.apache.commons.lang3.StringUtils

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

import java.util.concurrent.TimeUnit

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
            tablTabl "${g.render(template: 'annotationTmpl/tablTmpl', model: [experiment: experiment, category: eMeta.mdCategory])}"
        }
    }

    def axMetaActionChange(ExperimentMetadata eMeta){
//      println "axMetaActionChange ${params.colAction}  emetaData=${eMeta.dump()}!"
      switch (params.colAction){
        case 'Delete': delCol(eMeta)
                       return
          break

        case 'HideColumn': hideCol(eMeta)
                     return
          break

        case ['HideFromFilter','ShowOnFilter']: toggleShowOnFilter(eMeta)
                     return
          break

//        case 'ShowOnFilter': hideCol(eMeta)
//                     return
//          break

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
      experiment.expMetadatas.each{ eMeta ->
        setColVisability(eMeta, true)
      }
      println "cat ${params?.category}"
      ExperimentMetadataCategory categy = ExperimentMetadataCategory.get(params?.category?.toLong())
      ExperimentMetadata eMeta = ExperimentMetadata.findAllByExperimentAndMdCategory(experiment,categy).first()
//      println "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: eMeta.mdCategory])}"
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: experiment, category: eMeta.mdCategory])}"
      }
    }

    def toggleShowOnFilter(ExperimentMetadata eMeta){
      println "toggle filter flag ${eMeta.mdKey}"
      eMeta.dispOnFilter = !eMeta.dispOnFilter
      eMeta.save flush: true
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: eMeta.experiment, category: eMeta.mdCategory])}"
      }
    }

    def hideCol(ExperimentMetadata eMeta){
      println "hide ${eMeta.mdKey}"
      setColVisability(eMeta, false)
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: eMeta.experiment, category: eMeta.mdCategory])}"
      }
    }

    def setColVisability(ExperimentMetadata eMeta, Boolean visible){
      eMeta.visible = visible
      eMeta.save flush: true
    }



    def delCol(ExperimentMetadata eMeta){
      println "delete eMeta ${eMeta.mdKey}"
      eMeta.experiment.expFiles.each{ expFile ->
          expFile.metaDatas.findAll{ it.mdKey == eMeta.mdKey }.sort{it.id}.each { expFileMd ->
              expFileMd.expFile.metaDatas.remove(expFileMd)
              expFileMd.delete flush: true
          }
      }
      eMeta.mdVals*.delete()
      eMeta.mdVals.clear()
      eMeta.delete flush: true
      render (contentType:"text/json") {
        success true
        tablTabl "${g.render(template: 'annMasterTbl', model: [experiment: eMeta.experiment, category: eMeta.mdCategory])}"
      }
    }

    def addColumn(Experiment experiment){ //add metaData column in annotation table
        List<ExperimentMetadata> existingEMetaData = ExperimentMetadata.findAllByExperimentAndMdKey(experiment, params.mdKey)

        if(existingEMetaData && existingEMetaData.size() > 0) {
            flash.error = "Attribute " + params.mdKey + " exists for the experiment."
        } else {

            params.experiment = experiment
            params.dispOrder = utilsService.getLowestUniqueDispOrder(experiment)
            ExperimentMetadata eMetaData = new ExperimentMetadata(params)
            eMetaData.save(flush: true)
            params.expMetaData = eMetaData
            List<String> valueList = params.list('mdValue')
//        println "addColumn valueList ${valueList} size ${valueList.size()}"
            Integer vListSize = valueList.size()
            valueList.each { pmdValue ->
                ExperimentMetadataValue eMetaValue = new ExperimentMetadataValue(expMetaData: eMetaData, mdValue: pmdValue, mdType: vListSize > 1 ? 'List' : 'String', dispOrder: 1)
                eMetaValue.save(flush: true)
                if (eMetaData?.mdVals) {
                    eMetaData?.mdVals?.add(eMetaValue)
                } else {
                    eMetaData?.mdVals = [eMetaValue]
                }
            }

            /*
    //      does not work 100%
            params.mdValue.each{ mdValue ->
                ExperimentMetadataValue eMetaValue = ExperimentMetadataValue.findOrSaveByExpMetaDataAndMdValue(eMetaData, mdValue)
                params.mdType = params.mdType.size()>1 ? 'List' : 'String'
                params.mdValue = mdValue
                eMetaValue.properties = params
                eMetaValue.save()
                if(eMetaData?.mdVals){
                    eMetaData?.mdVals?.add(eMetaValue)
                }
                else{
                    eMetaData?.mdVals = [eMetaValue]
                }
            }
            */

            println "add metaData column  ${eMetaData?.mdVals}"
        }
        redirect action: 'annotationTbl', id: experiment.id
    }

    def exportAnnotationTempl(Experiment experiment) {
        String separator = ","
        String expFileName = "annotationTempl_${experiment?.id}_${new Date().format('yy-MM-dd__hh_mm')}.csv"
        String expTempl = "FCS File Name${separator}\n"
        experiment?.expFiles?.sort { it.fileName }?.each { fcsFile ->
            expTempl += "${fcsFile?.fileName}${separator}\n"
        }
        response.setHeader("Content-disposition", "attachment; filename=${expFileName}")
        render contentType: "text/csv", text: "${expTempl}"
    }

    def addCategory(Experiment experiment){ //add metaData column in annotation table
        params.experiment = experiment
        params.visible = true
        params.dispOnFilter = true
        params.isDefault = false
        ExperimentMetadataCategory emCategory = ExperimentMetadataCategory.findOrSaveByExperimentAndMdCategory(experiment, params.mdCategory)
        emCategory.properties = params
        emCategory.save(flush: true)
        redirect action: 'annotationTbl', id: experiment.id
    }

    def updateCategory(ExperimentMetadataCategory category){ //add metaData column in annotation table
        if(category == null){
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if(category.hasErrors()){
            transactionStatus.setRollbackOnly()
            respond category.errors, action: 'annotationTbl', id: category.experiment.id
            return
        }
        params.visible = true
        params.dispOnFilter = true
        params.isDefault = false
        category.properties = params
        category.save(flush: true)
        redirect action: 'annotationTbl', id: category.experiment.id
    }

    @Transactional
    def deleteCategory(ExperimentMetadataCategory category ) {
        if (category == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (ExperimentMetadata.findAllByMdCategory(category).size()>0){
            def defaultCategory = ExperimentMetadataCategory.findOrSaveByExperimentAndMdCategory(category.experiment, 'Basics')
            ExperimentMetadata.findAllByMdCategory(category).each { eMeta ->
                eMeta.mdCategory = defaultCategory
                eMeta.mdCategory.visible = true
                eMeta.save flush: true
            }
        }
        category.delete flush: true
        redirect action: 'annotationTbl', id: category.experiment.id
    }

    def axEditCategory(Experiment experiment){
        ExperimentMetadataCategory category = ExperimentMetadataCategory.get(params.catId.toLong())
//        println " axEditCategory exp=${experiment?.id}"
//        println "${g.render (template: 'annotationTmpl/categoryEditModal', model: [experiment: experiment, category: category])}"
        render (contentType:"text/json") {
            success true
            catModalTmpl "${g.render (template: 'annotationTmpl/categoryEditModal', model: [experiment: experiment, category: category])}"
        }
    }

    def editColumn(Experiment experiment){ //edit metaData column in annotation table
        println "${params.metaValId.value.size()}"
        ExperimentMetadata eMetaData = ExperimentMetadata.get(params?.metaId?.toLong())
        eMetaData.properties = params
        eMetaData.save(flush: true)
        def mdValList = eMetaData.mdVals
        eMetaData.mdVals = []
        mdValList.each{ mdValue ->
            mdValue.discard()
            mdValue.delete(flush: true)
        }
        List<String> valueList = params.list('mdValue')
//        println "addColumn valueList ${valueList} size ${valueList.size()}"
        Integer vListSize = valueList.size()
        def dispOrder = params.dispOrder
        int order = 1 //0 is used for experimentmetadata
        valueList.each{ pmdValue ->
            ExperimentMetadataValue eMetaValue = new ExperimentMetadataValue(expMetaData:eMetaData, mdValue: pmdValue, mdType: vListSize>1 ? 'List' : 'String', dispOrder: dispOrder[order++])
            eMetaValue.save(flush: true)
            if(eMetaData?.mdVals){
                eMetaData?.mdVals?.add(eMetaValue)
            }
            else{
                eMetaData?.mdVals = [eMetaValue]
            }
        }
        println 'edit metaData column'
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
    def deleteMultiple() {
        try {
            def fileIds= JSON.parse(params.fileIds)
            for(String id : fileIds) {
                def expFile = ExpFile.findById(id);
                File file = new File(expFile.getFilePath() + File.separator + expFile.getFileName());
                file.delete();
                erase(expFile)
            }
        } catch (Exception e) {
            render (contentType:"text/json") {
                success false
                msg e.getMessage()
            }
        }

        render (contentType:"text/json") {
            success true
            fileCount ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(params.expId.toLong(), true), true).size()
            msg "Files have been deleted!"
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

        def datasets = Dataset.withCriteria { expFiles { eq('id', expFile.id) } }
        for (Dataset dataset : datasets) {
            dataset.expFiles.removeElement(expFile)

            if (dataset.expFiles.size() == 0) {
                dataset.isActive = false
            }

            dataset.save flush: true
        }

        expFile.delete flush:true

        /*request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'expFile.label', default: 'Exp. File'), fcsFileContainer.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }*/
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
            ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        def requestedFile = request.getFile("annotationFile")
        File annotationFile = requestedFile.part.fileItem.tempFile
        if(annotationFile.size()<1){
            println "error: no file selected"
            flash.msg = "Error: No file selected!"
            doAnnotate(experiment)
            return
        }
        def lines = annotationFile.readLines();
        def separatorCount = lines.stream()
                .map({ l -> return StringUtils.countMatches(l, ",") })
                .filter({ l -> l != null })
                .distinct()
                .toArray();

        //csv file if same count of comma and has comma
        String separator = (separatorCount.length == 1 && separatorCount[0] > 0) ? "," : "\t"

        def headers = []
        def rows = []
        Integer lineCntr = 0
        annotationFile.splitEachLine("${separator}"){ fields ->
            if(lineCntr == 0){
                headers = fields
            }
            else{
                rows += [fields]
            }
            lineCntr += 1
        }

        def myMap = []
        rows.each { row ->
            myMap += [[headers, row].transpose().collectEntries()]
        }

        def categoryId = params?.categoryId
        println 'Selected category id: ' + categoryId

        utilsService.csvMetadataParse(experiment, categoryId, myMap, headers)
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
                            throw new Exception("File with the same name already exists")

                        def sha1 = chkSumService.getSha1sum(fcsFile.part.fileItem.tempFile)
                        def md5 = chkSumService.getMD5sum(fcsFile.part.fileItem.tempFile)
                        if(ExpFile.findAllByChkSumAndExperiment(sha1, experiment))
                            throw new Exception("File with the same checksum (${sha1}) already exists")

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
//        println "set session init values ${experiment}"
        if(experiment && experiment.expMetadatas) {
            experiment.expMetadatas.sort { it.mdCategory?.dispOrder }?.each { catgy ->
                experiment.expMetadatas.findAll { it.mdCategory == catgy.mdCategory }?.sort { it.dispOrder }?.each { eMeta ->
                    if (eMeta.mdVals.size() > 1)
                        session["meta_${eMeta.mdKey}"] = eMeta.mdVals.sort { it.dispOrder }.first().mdValue
                    else
                        session["meta_${eMeta.mdKey}"] = eMeta.mdVals.mdValue.join(',')
                }
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
                efMetaData.delete()
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

    def renderFCSFileInfo(ExpFile expFile) {
        if (expFile.reagentPanel == null) {
            def fcsFilePath = expFile.filePath + File.separator + expFile.fileName

            def fcsFile = new File(fcsFilePath)

            if (fcsFile.isFile()) {
                fcsService.readFile(fcsFile, false)

                def reagentPanel = new ReagentPanel()
                reagentPanel.reagentPanelName = "${expFile.fileName}-reagentPanel"
                def channelIds = []
                def transforms = []

                for (def i = 0; i < fcsService.channelShortname.length; i++) {
                    def channelShortname = fcsService.channelShortname[i]
                    def channelName = fcsService.channelName[i]
                    def channelId = fcsService.settings.find { it.value == channelShortname }?.key
                    channelId = channelId.substring(1, channelId.length() - 1);
                    channelIds.add(channelId)
                    transforms.add("")

                    def reagent = new Reagent()
                    reagent.dispOrder = i + 1
                    reagent.channel = channelShortname
                    reagent.marker = channelName
                    reagent.alias = channelId

                    reagentPanel.addToReagents(reagent)
                }

                User user = springSecurityService.currentUser
                reagentPanel.user = user

                expFile.reagentPanel = reagentPanel

                reagentPanel.validate()
                if (reagentPanel.hasErrors()) {
                    log.warn "Failed to save expFile $reagentPanel: $reagentPanel.errors"
                }

                expFile.save flush: true

                def transformationParameterList = TransformationParameter.findAllByIsPredefined(true)

                render(contentType: "text/json") {
                    success true
                    modelContent "${g.render(template: 'fcsFileParamTmpl', model: [id: expFile.id, name: fcsService.sampleName, parameters: fcsService.channelShortname, labels: fcsService.channelName, channelIds: channelIds, transforms: transforms, predefinedList: transformationParameterList])}"
                }
            } else {
                render(contentType: "text/json") {
                    success false
                    msg "FCS file does not exist!"
                }
            }
        } else {
            def reagents = expFile.reagentPanel.reagents

            def channelShortnames = []
            def channelNames = []
            def channelIds = []
            def transforms = []
            for (def reagent : reagents) {
                channelShortnames.add(reagent.channel)
                channelNames.add(reagent.marker)
                channelIds.add(reagent.alias)

                if(reagent.transformationParameter == null) {
                    transforms.add(new TransformationParameter())
                } else {
                    def jsonObj = "{\"transformName\": ${reagent.transformationParameter.transformName}," +
                            "\"transformType\": ${reagent.transformationParameter.transformType}," +
                            "\"parameterValues\": ${reagent.transformationParameter.parameterValues}," +
                            "\"isPredefined\": ${reagent.transformationParameter.isPredefined}}"
                    transforms.add(JSON.parse(jsonObj))
                }
                //transforms.add(reagent.transformationParameter == null ? new TransformationParameter() : reagent.transformationParameter)
            }

            def transformationParameterList = TransformationParameter.findAllByIsPredefined(true)

            render(contentType: "text/json") {
                success true
                modelContent "${g.render(template: 'fcsFileParamTmpl', model: [id: expFile.id, name: expFile.fileName, parameters: channelShortnames, labels: channelNames, channelIds: channelIds, transforms: transforms, predefinedList: transformationParameterList])}"
            }
        }
    }

    def processGateDrawing(ExpFile expFile) {
        if (expFile == null) {
            notFound()
            return
        }

        //It doesn't work if you use this and render in related if statement
        def fcsDrupalId

        //TODO: handle if there are multiple clinical sites
        def drupalServer = AnalysisServer.findByPlatform(3);
        def experiment = expFile.experiment;
        def fcsFilePath = expFile.filePath + File.separator + expFile.fileName;

        def dirLocation = grailsApplication.config.getProperty('clinical.dir', String)
        def foldershareScriptPath = grailsApplication.config.getProperty('clinical.foldershare', String)
        def phpLocation = grailsApplication.config.getProperty('clinical.php', String)

        def lsCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw ls "
        def mkdirCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw --format auto mkdir "
        def uploadCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw --format auto upload $fcsFilePath /$experiment.project.id/$experiment.id"
        def statCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw stat /$experiment.project.id/$experiment.id/$expFile.fileName"

        File dir = new File(dirLocation)

        //Check if FCS file exists in drupal site
        Process process = Runtime.getRuntime().exec(statCommand, null, dir)
        def out = new StringBuffer()
        def err = new StringBuffer()
        process.consumeProcessOutput(out, err)
        process.waitFor(10, TimeUnit.SECONDS)

        //Create FCS file if doesnt exist
        if (err.size() > 0) {
            println "Upload FCS File Error: $err"
            err.setLength(0);

            process = Runtime.getRuntime().exec(lsCommand + "/$experiment.project.id", null, dir)
            process.consumeProcessOutput(out, err)
            process.waitFor(10, TimeUnit.SECONDS)

            if (out.size() > 0) {
                println "LS Project Folder Output: $out"
                out.setLength(0);
            }

            //Create project folder if not exist
            if (err.size() > 0) {
                println "LS Project Folder Error: $err"
                err.setLength(0);

                process = Runtime.getRuntime().exec(mkdirCommand + "/$experiment.project.id", null, dir)
                process.consumeProcessOutput(out, err)
                process.waitFor(10, TimeUnit.SECONDS)

                if (out.size() > 0) {
                    println "MKDIR Project Folder Output: $out"
                    out.setLength(0);
                }

                if (err.size() > 0) {
                    println "MKDIR Project Folder Error: $err"
                    err.setLength(0);
                }
            }

            process = Runtime.getRuntime().exec(lsCommand + "/$experiment.project.id/$experiment.id", null, dir)
            process.consumeProcessOutput(out, err)
            process.waitFor(10, TimeUnit.SECONDS)

            if (out.size() > 0) {
                println "LS Experiment Folder Output: $out"
                out.setLength(0);
            }

            //Create experiment folder if not exist
            if (err.size() > 0) {
                println "LS Experiment Folder Error: $err"
                err.setLength(0);

                process = Runtime.getRuntime().exec(mkdirCommand + "/$experiment.project.id/$experiment.id", null, dir)
                process.consumeProcessOutput(out, err)
                process.waitFor(10, TimeUnit.SECONDS)

                if (out.size() > 0) {
                    println "MKDIR Experiment Folder Output: $out"
                    out.setLength(0);
                }

                if (err.size() > 0) {
                    println "MKDIR Experiment Folder Error: $err"
                    err.setLength(0);
                }
            }

            process = Runtime.getRuntime().exec(uploadCommand, null, dir)
            process.consumeProcessOutput(out, err)
            process.waitFor(10, TimeUnit.SECONDS)

            if (out.size() > 0) {
                println "Upload FCS File Output: $out"
                out.setLength(0);

                process = Runtime.getRuntime().exec(statCommand, null, dir)
                process.consumeProcessOutput(out, err)
                process.waitFor(10, TimeUnit.SECONDS)
            }
            if (err.size() > 0) println "Upload FCS File Error: $err"
        }

        if (out.size() > 0) {
            println "Stat FCS File Output: $out"
            //TODO Find a better way
            fcsDrupalId = out.substring(out.indexOf("ID: ") + 4);
            fcsDrupalId = fcsDrupalId.substring(0, fcsDrupalId.indexOf("ParentID"))
            fcsDrupalId = fcsDrupalId.trim()
            println "FCS file id in drupal: $fcsDrupalId"

            //Generate map file
            def transformation = JSON.parse(params?.transformation);
            def updateData = params?.updateData;

            if (updateData) {
                log.info"Updating parameter map data for $expFile.fileName"

                def reagents = expFile.reagentPanel.reagents

                for (def i = 0; i < reagents.size(); i++) {
                    def reagent = reagents[i]
                    def parameterName = reagent.channel
                    def markerName = reagent.marker

                    def transformationData = transformation.find { it -> it.name.equals(parameterName) }
                    reagent.marker = transformationData.longName

                    def defaultTransform = transformationData.defaultTransform
                    if (defaultTransform) {
                        def transformationParameter = reagent.transformationParameter == null ? new TransformationParameter() : reagent.transformationParameter

                        switch (defaultTransform.transformType) {
                            case "linear":
                                def a = defaultTransform.a
                                def t = defaultTransform.t
                                transformationParameter.parameterValues = "{\"a\":$a,\"t\":$t}"
                                transformationParameter.isPredefined = false
                                break;
                            case "log":
                                def m = defaultTransform.m
                                def t = defaultTransform.t
                                transformationParameter.parameterValues = "{\"m\":$m,\"t\":$t}"
                                transformationParameter.isPredefined = false
                                break;
                            case "logicle":
                                def a = defaultTransform.a
                                def t = defaultTransform.t
                                def m = defaultTransform.m
                                def w = defaultTransform.w
                                transformationParameter.parameterValues = "{\"a\":$a,\"t\":$t,\"m\":$m,\"w\":$w}"
                                transformationParameter.isPredefined = false
                                break;
                            case "predefined" :
                                reagent.transformationParameter = TransformationParameter.findByTransformName(defaultTransform.name)
                                //Add predefined parameter values to parameter mapping file
                                def values = new JsonSlurper().parseText(reagent.transformationParameter.parameterValues)
                                values.each { key, value ->
                                    transformationData.defaultTransform[key] = value
                                }

                                break;
                        }

                        if (!defaultTransform.transformType.equals("predefined")) {
                            transformationParameter.transformName = "${expFile.fileName}-tp"
                            transformationParameter.transformType = defaultTransform.transformType
                            transformationParameter.save flush: true
                            reagent.transformationParameter = transformationParameter
                        } else {
                            defaultTransform.transformType = reagent.transformationParameter.transformType
                            defaultTransform.remove("name")
                        }

                        reagent.transformName = defaultTransform.transformType
                    }
                }

                expFile.save flush: true
            }

            def builder = new groovy.json.JsonBuilder()
            builder {
                version "FlowGateJsonParameterMap_1.0"
                name "parameter_flowgate"
                creatorSoftware "FlowGate Parameter Map version 1.0.0"
                parameters transformation
            }

            def mapFileLocation = "${dirLocation}/${expFile.fileName.split(".fcs")[0]}_parameter.map"
            new File(mapFileLocation).write(builder.toPrettyString())
            def mapUploadCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw --format auto upload $mapFileLocation /$experiment.project.id/$experiment.id"

            process = Runtime.getRuntime().exec(mapUploadCommand, null, dir)
            process.consumeProcessOutput(out, err)
            process.waitFor(10, TimeUnit.SECONDS)

            if (out.size() > 0) println "Upload FCS File Output: $out"
            if (err.size() > 0) println "Upload FCS File Error: $err"

            render(contentType: "text/json") {
                success true
                url "http://$drupalServer.url/flowgate_clinical/$fcsDrupalId/editgating"
            }
        } else {
            render(contentType: "text/json") {
                success false
                msg "Error occured!"
            }
        }
    }
}
