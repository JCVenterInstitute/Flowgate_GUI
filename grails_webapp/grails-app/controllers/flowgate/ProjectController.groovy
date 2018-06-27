package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(['IS_AUTHENTICATED_FULLY'])
@Transactional
class ProjectController {

//    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE"]
    static allowedMethods = [save: "POST", update: "PUT"]
    static defaultAction = "list"

    def springSecurityService
    def utilsService

//    @Secured(['ROLE_Admin','ROLE_User','ROLE_NewUser','ROLE_Guest','IS_AUTHENTICATED_ANONYMOUSLY', 'ROLE_ANONYMOUS','permitAll'])
    def axToggleView(){
        session?.projCardView = !session?.projCardView ?: false
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        if(session?.projCardView){
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projCardsTmpl', model: [projectList: projectList])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projListTmpl', model: [projectList: projectList])}"
            }
        }
    }

    def axToggleExpView(){
        if(session?.expCardView == null)
            session?.expCardView = true
        session?.expCardView = !session?.expCardView
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(Project.findByIdAndIsActive(session.projectOpenId?.toLong(), true),true, [params: params])
        if(session?.expCardView){
            render (contentType:"text/json"){
                success true
                expContent "${g.render(template: 'templates/expCardsTmpl', model: [experimentList: experimentList])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success true
                expContent "${g.render(template: 'templates/expListTmpl', model: [experimentList: experimentList])}"
            }
        }
    }

    def axSearch(){
        def project = Project.findByIdAndIsActive(session?.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        def searchLst = []
        projectList.findAll{it.title.contains(params?.filterString) ? searchLst.push(it.id) : '' }
        if(params?.filterString != ''){
            session.filterString = params?.filterString
            session.searchLst = searchLst
        }
        if(session?.projCardView){
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projCardsTmpl', model: [projectList: projectList])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projListTmpl', model: [projectList: projectList])}"
            }
        }

    }

    def axClearSearch(){
        def project = Project.findByIdAndIsActive(session?.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        def searchLst = []
//        projectList.findAll{it.title.contains(params?.filterString) ? searchLst.push(it.id) : '' }
        params.filterString = ''
        session.filterString = ''
        session.searchLst = []
        if(session?.projCardView){
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projCardsTmpl', model: [projectList: projectList])}"
            }
        }
        else {
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projListTmpl', model: [projectList: projectList])}"
            }
        }

    }


    def axSearchTree(){
        def project = Project.findByIdAndIsActive(session?.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        def searchLst = []
        projectList.findAll{it.title.contains(params?.filterString) ? searchLst.push(it.id) : '' }
        if(params?.filterString != ''){
            session.filterString = params?.filterString
            session.searchLst = searchLst
        }
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
        }

    }

    def axClearSearchTree(){
        def project = Project.findByIdAndIsActive(session?.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        def searchLst = []
//        projectList.findAll{it.title.contains(params?.filterString) ? searchLst.push(it.id) : '' }
        params.filterString = ''
        session.filterString = ''
        session.searchLst = []
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList, searchLst: searchLst])}"
        }

    }

    def axProjectClick(){
        session.projectOpenId = params.projId.toLong()
        Project openProject = Project.findByIdAndIsActive(session.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(openProject, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
            contentBtnBar "${g.render(template: 'templates/indexTopBtnBar', model: [project: openProject])}"
            contentPage "${g.render(template: 'templates/indexTmpl', model: [project: openProject, experimentList: experimentList])}"
        }
    }

    def axCloneProjectClick() {
        Project sourceProject = Project.findByIdAndIsActive(params.projId.toLong(), true)
        utilsService.clone('project', sourceProject, null, true, '-clone')
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(sourceProject, true)
        render (contentType:"text/json"){
            success true
            contentTree "${g.render(template: '/shared/treeView', model: [projectList: projectList, experimentList: experimentList])}"
            contentPage "${g.render(template: 'templates/indexTmpl', model: [project: sourceProject, experimentList: experimentList])}"
        }
    }

    def manageUsers(){
        Project project = Project.get(params?.id?.toLong())
        utilsService.doManage(project, params?.owners*.toLong() as List<Long>, 'owner')
        utilsService.doManage(project, params?.members*.toLong() as List<Long>, 'member')
        redirect action: 'index', params:[pId: params?.id]
    }

    def manageExpUsers(){
        Experiment experiment = Experiment.get(params?.id?.toLong())
        utilsService.doManage(experiment, params?.owners*.toLong() as List<Long>, 'owner')
        utilsService.doManage(experiment, params?.members*.toLong() as List<Long>, 'member')
        redirect action: 'index', params:[pId: experiment.project.id]
    }

    @Secured(['ROLE_Admin','ROLE_User'])
    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        User user = springSecurityService.currentUser
        if(params?.pId)
            session.projectOpenId = params?.pId
        Project openProject = Project.findByIdAndIsActive(session.projectOpenId, true)
        ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(openProject, true)
        respond projectList, model:[project: openProject, projectCount: projectList.size(), experimentList: experimentList]
    }

//    @Secured(['ROLE_Admin','ROLE_User','ROLE_NewUser','ROLE_Guest','IS_AUTHENTICATED_ANONYMOUSLY', 'ROLE_ANONYMOUS','permitAll'])
//    @Secured(['IS_AUTHENTICATED_ANONYMOUSLY'])
    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        User user = springSecurityService.currentUser

        def projectList = utilsService.getProjectListForUser(user, params)

        respond projectList, model: [projectCount: projectList.size()]
    }

    def show(Project project) {
        respond project
    }

    def create() {
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        Project project = new Project(params)
        respond project, model: [projectList: projectList, projectCount: projectList.size(), experimentList:[] ]
    }

    @Transactional
    def save(Project project) {
        User owner = springSecurityService.currentUser
        if (project == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (project.hasErrors()) {
            println "project has errors!"
            transactionStatus.setRollbackOnly()
            respond project.errors, view:'create'
            return
        }
        project.save flush:true
        ProjectUser.create(project, owner, 'owner')
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'project.label', default: 'Project'), "${project.title.take(10)+'... '}"])
                redirect view: 'index'
            }
            '*' { respond project, [status: CREATED] }
        }
    }

    def edit(Project project) {
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params)
        respond project, model: [projectList: projectList]
    }

    @Transactional
    def update(Project project) {
        if (project == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (project.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond project.errors, view:'edit'
            return
        }
        project.save flush:true
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'project.label', default: 'Project'), project.title])
//                redirect project
                redirect view: 'index'
            }
            '*'{ respond project, [status: OK] }
        }
    }

    @Transactional
    def delete(Project project) {
        if (project == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
//        TODO maybe remove the conditional check, this project should always be the open one
        if(session?.projectOpenId == project.id){
            session?.projectOpenId = 0
        }
        project.isActive = false
        project.save flush:true
        flash.message = message(code: 'default.deleted.message', args: [message(code: 'project.label', default: 'Project'), project.title])
        redirect view: 'index'
    }

    //  for Super Admins to completely erase experiments from db with their expFiles
    @Transactional
    def erase(Project project) {
//        TODO remove ExperimentUser / if not done automatically
        if (project == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        ProjectUser.where { project == project }.deleteAll()
        Project orphanProject = Project.findByTitle('orphanProject')
        project.experiments.each {
            it.project = orphanProject
            it.save flush:true
        }
        project.delete flush:true
//        flash.message = message(code: 'default.deleted.message', args: [message(code: 'project.label', default: 'Project'), project.id])
//        redirect view: 'index'

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'project.label', default: 'Project'), project.title])
//                redirect action:"index", method:"GET"
                redirect view: 'index'
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'project.label', default: 'Project'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
