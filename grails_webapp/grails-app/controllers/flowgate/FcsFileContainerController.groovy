package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(['ROLE_Admin','ROLE_User'])
@Transactional(readOnly = true)
class FcsFileContainerController {

    static allowedMethods = [save: "POST", update: "PUT", axDeleteContainer: "DELETE"]

    def springSecurityService
    def utilsService

    /*def axFcsContainerToggleClick() {
        def experiment = Experiment.findByIdAndIsActive(params.eId.toLong(), true)
        def fcsContainer = FcsFileContainer.findByIdAndIsActive(params.fcsContainerId.toLong(), true)
        if(experiment && fcsContainer){
            if(session.fcsContainerOpenIds?.contains(fcsContainer.id)){
                session.fcsContainerOpenIds?.remove(session.fcsContainerOpenIds?.indexOf(fcsContainer.id))
            }
            else {
                session.fcsContainerOpenIds?.push(fcsContainer.id)
            }
//            fcsContainer.save(failOnSafe: true, flush: true)
//            TODO catch the save error
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(experiment.project, true)
            render (contentType:"text/json"){
                success true
                contentTree "${g.render( template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render( template: 'fcsContainerTmpl', model: [experiment: experiment, fcsContainer: fcsContainer])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success false
                msg "${g.message(message: 'error.experiment.noActive.message')}"
            }
        }
    }

//    TODO do collapseAll and expandAll with contolled by a param in one method
    def collapseAll(){
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        if(experiment){
            FcsFileContainer.findAllByExperimentAndIsActive(experiment, true).each {
                if(session?.fcsContainerOpenIds?.contains(it.id)) session?.fcsContainerOpenIds?.remove(session?.fcsContainerOpenIds?.indexOf(it.id))
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

    //    TODO do collapseAll and expandAll with contolled by a param in one method
    def expandAll(){
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        if(experiment){
            FcsFileContainer.findAllByExperimentAndIsActive(experiment, true).each {
                if(!session?.fcsContainerOpenIds?.contains(it.id)) session?.fcsContainerOpenIds?.push(it.id)
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

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond FcsFileContainer.list(params), model:[fcsFileContainerCount: FcsFileContainer.count()]
    }

    def show(FcsFileContainer fcsFileContainer) {
        respond fcsFileContainer
    }

    def axFileContainerCreate(){
        FcsFileContainer fcsFileContainer = new FcsFileContainer()
        fcsFileContainer.experiment = Experiment.get(params?.eId?.toLong())
        fcsFileContainer.isActive = true
//        TODO get unique name
        fcsFileContainer.title = 'new fcs container'
        Boolean saveOk = fcsFileContainer.save flush:true
        if(saveOk) {
            session.fcsContainerOpenIds?.push(fcsFileContainer.id)
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            Project project = fcsFileContainer.experiment.project
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
            flash.message = 'fcs container successfully added'
            render (contentType:"text/json") {
                success true
                contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: fcsFileContainer.experiment])}"
            }
        }
        else {
            flash.message = 'something wrong with creating new fcs container! please check the logs'
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
    def axDeleteContainer() {
        FcsFileContainer fcsFileContainer = FcsFileContainer.get(params?.fcsContainerId?.toLong())
        if (fcsFileContainer == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        fcsFileContainer.isActive = false
        Boolean saveOk = fcsFileContainer.save flush:true
        if(saveOk) {
            ArrayList<Project> projectList = Project.findAllByIsActive(true)
            Project project = fcsFileContainer.experiment.project
            ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
            render (contentType:"text/json") {
                success true
                contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
                contentPage "${g.render(template: '/experiment/templates/indexTmpl', model: [experiment: fcsFileContainer.experiment])}"
            }
        }
        else {
            flash.message = message(code: 'default.delete.error.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id], default: 'error deleting container! ')
            render (contentType:"text/json") {
//                TODO check what templates to render in case of error
                success false
//                contentBtnBar flash.message
//                contentTree flash.message
//                contentPage flash.message
            }
        }

//        redirect controller: 'experiment', view: 'index', params: [pId: fcsFileContainer.experiment.project.id, eId: fcsFileContainer.experiment.id]
        *//*request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }*//*
    }


    def create() {
        respond new FcsFileContainer(params)
    }

    @Transactional
    def save(FcsFileContainer fcsFileContainer) {
        if (fcsFileContainer == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (fcsFileContainer.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond fcsFileContainer.errors, view:'create'
            return
        }

        fcsFileContainer.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
                redirect fcsFileContainer
            }
            '*' { respond fcsFileContainer, [status: CREATED] }
        }
    }

    def edit(FcsFileContainer fcsFileContainer) {
        respond fcsFileContainer
    }

    @Transactional
    def update(FcsFileContainer fcsFileContainer) {
        if (fcsFileContainer == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (fcsFileContainer.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond fcsFileContainer.errors, view:'edit'
            return
        }

        fcsFileContainer.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
                redirect fcsFileContainer
            }
            '*'{ respond fcsFileContainer, [status: OK] }
        }
    }

    @Transactional
    def delete(FcsFileContainer fcsFileContainer) {
        if (fcsFileContainer == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

//        fcsFileContainer.delete flush:true
        fcsFileContainer.isActive = false
        fcsFileContainer.save flush:true
        flash.message = message(code: 'default.deleted.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
        //TODO check for flickering / maybe do with ajax action
        redirect controller: 'experiment', view: 'index', params: [pId: fcsFileContainer.experiment.project.id, eId: fcsFileContainer.experiment.id]
        *//*request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }*//*
    }

    @Transactional
    def erase(FcsFileContainer fcsFileContainer) {

        if (fcsFileContainer == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        fcsFileContainer.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'fcsFileContainer.label', default: 'FcsFileContainer'), fcsFileContainer.id])
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
    }*/
}
