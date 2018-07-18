package flowgate

import grails.plugin.springsecurity.annotation.Secured
import grails.transaction.Transactional

import static org.springframework.http.HttpStatus.*

@Secured(['ROLE_Admin','ROLE_User'])
@Transactional(readOnly = true)
class ExperimentBackupController {

    static allowedMethods = [save: "POST", update: "PUT", axCloneExperimentClick: "PUT", index: "GET"]

    def springSecurityService
    def utilsService

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
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            params.max = Math.min(max ?: 10, 100)
            render view: 'index', model: [project: experiment?.project, projectList: projectList, experiment: experiment, experimentList: experimentList]
        }
        else{
            if(experiment?.project){
                redirect controller: 'project', view: 'index'
            }
            else{
                render view: '/error2', model: [message: g.message(message: 'error.experiment.noActive.message')]
            }
        }
    }

    def show(Experiment experiment) {
        respond experiment
    }

    def create() {
        ArrayList<Project> projectList = Project.findAllByIsActive(true, [params: params])
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(Project.get(params?.pId?.toLong()), true)
        respond new Experiment(params), model: [owner: springSecurityService.currentUser, projectList: projectList, pId: params?.pId, experimentList: experimentList
        ]
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
            respond experiment.errors, view:'create'
            return
        }
        experiment.save flush:true
        ExperimentUser.create(experiment, springSecurityService.currentUser, 'owner')
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.id])
//                redirect experiment
                redirect view: 'index'
            }
            '*' { respond experiment, [status: CREATED] }
        }
    }

    def edit(Experiment experiment) {
        respond experiment
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
            respond experiment.errors, view:'edit'
            return
        }
        experiment.save flush:true
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.id])
                redirect experiment
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
        flash.message = message(code: 'default.deleted.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.id])
        redirect view: 'index', params: [pId: experiment.project.id]
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
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'experiment.label', default: 'Experiment'), experiment.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'experiment.label', default: 'Experiment'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
