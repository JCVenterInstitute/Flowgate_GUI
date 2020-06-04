package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(['ROLE_Admin','ROLE_User'])
//@Transactional(readOnly = true)
@Transactional
class ExperimentController {

    static allowedMethods = [save: "POST", manageUsers: "POST", update: ["PUT","POST"], axCloneExperimentClick: "PUT", index: "GET"]

    def springSecurityService
    def utilsService

    @Secured(['IS_AUTHENTICATED_ANONYMOUSLY', 'ROLE_ANONYMOUS','permitAll'])
    def renderAnalysisHtml(){
//        def htmlContent = new File('../analysis/results/analysisExample/analysisExample.html').text
//        def htmlContent = new File('/analysis/anex.html').text
//        render text: htmlContent, contentType: 'text/html', encoding: 'UTF-8'
        File myFile = new File('/Users/acs/Sources/flowgate/grails-app/views/analysis/results/DAFI-tsne.html')
        render file: myFile, contentType: 'text/html'
    }

    def renderAnalysisHtml2(){
        File myFile = new File('/Users/acs/Sources/flowgate/grails-app/views/analysis/results/UCSD_CLL_New.html')
        render file: myFile, contentType: 'text/html'
    }

    def axExperimentClick(){
        Project project = Project.findByIdAndIsActive(params?.projId?.toLong(), true)
//        closeAllExcept(params.expId.toLong())
        def experiment = Experiment.findByIdAndIsActive(params?.expId?.toLong(), true)
//        if(experiment && !experiment.isOpen){
//            experiment.isOpen = true
//            experiment.save(failOnSafe: true, flush: true)
//        }
//        TODO check if no experiment is found!!
        session.experimentOpenId = experiment.id
        session.experimentEditModeId = experiment.id
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        ArrayList<Project> projectList = Project.findAllByIsActive(true)
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
            contentPage "${g.render(template: 'templates/indexTmpl', model: [experiment: experiment])}"
        }
    }

    def axCloneExperimentClick() {
        Experiment sourceExperiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        utilsService.clone('experiment', sourceExperiment, sourceExperiment.project, true, '-clone')
        ArrayList<Project> projectList = Project.findAllByIsActive(true, [params: params])
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(sourceExperiment.project, true)
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
            contentPage "${g.render(template: 'templates/indexTmpl', model: [experiment: sourceExperiment])}"
        }
    }

    def axExperimentTitleSave(){
        // TODO check experiment should be active, no need to proofe
        Experiment experiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        if(experiment){
            experiment.title = params.titleValue ?: ''
            Boolean saveOk = experiment.save(failOnSafe: true, flush: true)
            ArrayList<Project> projectList = Project.findAllByIsActive(true, [params: params])
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            render (contentType:"text/json") {
                success saveOk
                contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
            }
        }
        else{
            //no active experiment found!
            render (contentType:"text/json") {
                success false
                msg "${g.message( message: 'error.experiment.noActive.message')}"
            }
        }
    }

    def axExperimentDescriptionSave(){
        // TODO check experiment should be active, no need to proofe
        Experiment experiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        if(experiment){
            experiment.description = params.descriptionValue ?: ''
            Boolean saveOk = experiment.save(failOnSafe: true, flush: true)
            render (contentType:"text/json") {
                success saveOk
            }
        }
        else{
            //no active experiment found!
            render (contentType:"text/json") {
                success false
                msg "${g.message( message: 'error.experiment.noActive.message')}"
            }
        }

    }

    def toggleExperimentEditModeClick() {
        def experiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        if(experiment){
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            session.experimentEditModeId = session.experimentEditModeId == experiment.id ? 0 : experiment.id
            session.experimentOpenId = experiment.id
            //            TODO persist openstatus in userSettings - do in service!
            render (contentType:"text/json"){
                success true
                contentBtnBar "${g.render(template: 'templates/indexTopBtnBar', model: [experiment: experiment])}"
                contentPage "${g.render(template: 'templates/indexTmpl', model: [experiment: experiment])}"
            }
        }
        else{
            render (contentType:"text/json"){
                success false
                msg "${g.message(message: 'error.experiment.noActive.message')}"
            }
        }
    }

    def manageUsers(){
        Experiment experiment = Experiment.get(params?.id?.toLong())
        utilsService.doManage(experiment, params?.owners*.toLong() as List<Long>, 'owner')
        utilsService.doManage(experiment, params?.members*.toLong() as List<Long>, 'member')
        redirect controller: 'experiment', action: 'index', params:[eId: params?.id]
    }

    def miFcytTbl(Experiment experiment){
        respond experiment
    }

    def exportMifCytTempl(Experiment experiment){
        String separator = ","
//        String mifcytFileName = "mifcytTempl_${experiment?.id}_${new Date().format('yy-MM-dd__hh_mm')}.csv"
        String mifcytFileName = "mifcytTempl.csv"
//        TODO save template in csv file
        String mifcytTempl = """Category${separator}Key${separator}Value
Experiment Overview${separator}Purpose/Goal/Hypothesis${separator}
Experiment Overview${separator}Experiment Variables${separator}
Experiment Overview${separator}Conclusions${separator}
Experiment Overview${separator}Quality Control${separator}
Flow Sample (Specimen)${separator}Material${separator}
Flow Sample (Specimen)${separator}Source/Organism/Location${separator}
Flow Sample (Specimen)${separator}Treatment${separator}
Flow Sample (Specimen)${separator}Reagent/Analyte/Detector/Reporter${separator}
Data Analysis${separator}List-mode Data${separator}
Data Analysis${separator}Compensation${separator}
Data Analysis${separator}Gating${separator}
Data Analysis${separator}Descriptive statistics${separator}
Instrument Details${separator}Instrument Identification${separator}
Instrument Details${separator}Fluidics Configuration${separator}
Instrument Details${separator}Optical Configuration${separator}
Instrument Details${separator}Electronic Configuration${separator}
                             """
        response.setHeader("Content-disposition", "attachment; filename=${mifcytFileName}")
        render contentType: "text/csv", text: "${mifcytTempl}"
    }

    def importMifcyt(Experiment experiment){
        String separator = params.separator ?: ","

        def miFcytCsvFile = request?.getFile("mifcytFile")
        File fcytCsvFile = miFcytCsvFile?.part?.fileItem?.tempFile
        if(fcytCsvFile.size()<1){
            println "error: no file selected"
            flash.msg = "Error: No file selected!"
            flash.messsage = "Error: No file selected!"
            redirect action: 'miFcytTbl', id: experiment.id
            return
        }
        def headers = []
        def rows = []
        Integer lineCntr = 0
        fcytCsvFile.splitEachLine("${separator}"){ fields ->
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
        utilsService.fcytMetadataParse(experiment, myMap, headers)
        render view: "miFcytTbl", model: [experiment: experiment]
    }

    def saveMiFlowData(Experiment experiment){
        if(!experiment){
            experiment = Experiment.findByIdAndIsActive(params?.eId?.toLong(), true)
        }
        ExperimentMetadata emD
        ExperimentMetadataValue emV
        params.findAll { it.key.startsWith('expMetaData-')}.each {
            emD = ExperimentMetadata.get((it.key-'expMetaData-').toLong())
            emV = ExperimentMetadataValue.findByExpMetaData(emD)
            if(it?.value){
                emV.mdValue = it?.value
                emV.save()
//                println "emV Errs ${emV.hasErrors()} metaDatStr = ${it} k:${it.key-'expMetaData-'} v:${it.value}"
            }
        }
        redirect action: 'index', params: [eId: experiment?.id]
    }


    def index(Integer max) {
//        TODO refactor pId not needed / use project from experiment
//        Project project = Project.findByIdAndIsActive(params?.pId?.toLong(), true)
        Experiment experiment = Experiment.findByIdAndIsActive(params?.eId?.toLong(), true)
        if(experiment){
//            TODO persist openstatus in userSettings - do in service!
            session.experimentOpenId = experiment?.id
            session.projectOpenId = experiment?.project?.id
//            experiment.save(failOnSafe: true, flush: true)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment?.project, true)
            User user = springSecurityService.currentUser
            ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
            params.max = Math.min(max ?: 10, 100)
            request.withFormat {
                'html' { render view: 'index', model: [project: experiment?.project, projectList: projectList, experiment: experiment, experimentList: experimentList]   }
                'json' {
//                    render(contentType: 'text/json') { 'this should go to gson view!'  }
                    respond Experiment.list(params), model:[experimentCount: Experiment.count()]
                }
            }

        }
        else {
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

    def show(Experiment experiment) {
//        println 'show exp'
        request.withFormat {
            'html' { respond experiment  }
            'json' { render(contentType: 'text/json') { 'this should go to gson view!'  }
            }
        }
    }

    def create() {
        User user = springSecurityService.currentUser
        ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(Project.get(params?.pId?.toLong()), true)
        respond new Experiment(params), model: [owner: springSecurityService.currentUser, projectList: projectList, pId: params?.pId, experimentList: experimentList]
    }

    @Transactional
    def save(Experiment experiment) {
        experiment.project = Project.findByIdAndIsActive(params?.pId?.toLong(), true)
        experiment.validate()
        if (experiment == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (experiment.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            ArrayList<Project> projectList = utilsService.getProjectListForUser(springSecurityService.currentUser, params, session?.showInactive ?: false)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(Project.get(params?.pId?.toLong()), true)
            respond experiment.errors, view:'create', model: [projectList: projectList, pId: params?.pId, experimentList: experimentList]
            return
        }
//        TODO replace hardcoded 'Basics' with config value
        new ExperimentMetadataCategory(mdCategory: 'Basics', experiment: experiment, visible: true, dispOnFilter: true, isDefault: false).save()
        experiment.save flush:true
        ExperimentUser.create(experiment, springSecurityService.currentUser, 'owner').save(flush: true)
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.title.take(20)+'... '])
//                redirect view: 'index', params:[eId: experiment.id]
                redirect controller: 'experiment', action: 'index', params:[eId: experiment.id]
            }
            '*' { respond experiment, [status: CREATED] }
        }
    }

    def edit(Experiment experiment) {
        ArrayList<Project> projectList = utilsService.getProjectListForUser(springSecurityService.currentUser, params, session?.showInactive ?: false)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment?.project, true)
        respond experiment, model: [projectList: projectList, experimentList: experimentList]
    }

    @Transactional
    def update(Experiment experiment) {
        if (experiment == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (experiment.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment?.project, true)
            ArrayList<Project> projectList = utilsService.getProjectListForUser(springSecurityService.currentUser, params, session?.showInactive ?: false)
            experiment.title = experiment.getPersistentValue("title")
            respond experiment.errors, view: 'edit', model: [projectList: projectList, experimentList: experimentList]
            return
        }
        experiment.save flush:true
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.title.take(20)+'... '])
                redirect controller: 'experiment', action: 'index', params:[eId: experiment.id]
            }
            '*'{ respond experiment, [status: OK] }
        }
    }

    @Transactional
    def delete(Experiment experiment) {
//        TODO remove ExperimentUser / if not done automatically
        if (experiment == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if(experiment.id == session?.experimentOpenId?.toLong()) {
            session?.experimentOpenId = 0
        }
        experiment.isActive = false
        experiment.save flush:true
        flash.message = message(code: 'default.deleted.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.title.take(20)+'... '])
        redirect controller: 'project', action: 'index', params: [pId: experiment.project.id]
    }

    // for Super Admins to completely erase experiments from db with their expFiles
    @Transactional
    def erase(Experiment experiment) {
//        TODO remove ExperimentUser / if not done automatically
        if (experiment == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        experiment.delete flush:true
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.title.take(20)+'... '])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.title.take(20)+'... '])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
