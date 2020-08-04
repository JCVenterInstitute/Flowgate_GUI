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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
        def searchLst =  projectList.findAll{it.title.toLowerCase().contains(params?.filterString.toLowerCase()) || it.description.toLowerCase().contains(params?.filterString.toLowerCase())}
        /*if(params?.filterString != ''){
            session.filterString = params?.filterString
            session.searchLst = searchLst
        }*/
        if (session?.projCardView) {
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projCardsTmpl', model: [projectList: searchLst, filterString: params?.filterString])}"
            }
        } else {
            render (contentType:"text/json"){
                success true
                contentPage "${g.render(template: 'templates/projListTmpl', model: [projectList: searchLst, filterString: params?.filterString])}"
            }
        }
    }

    def axClearSearch(){
        def project = Project.findByIdAndIsActive(session?.projectOpenId?.toLong(), true)
        ArrayList<Experiment> experimentList = Experiment.findAllByProjectAndIsActive(project, true)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        } else {
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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
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
        Project project
        if(params?.pId) {
            session.projectOpenId = params?.pId
            project = Project.get(params.pId)
        }
        if ( !utilsService.isAffil(project) ) {
            flash.message = "You are not allowed to access this project!"
            redirect action: 'list'
        } else {
            Project openProject

            if(session?.showInactive)
                openProject = Project.findById(session.projectOpenId)
            else
                openProject = Project.findByIdAndIsActive(session.projectOpenId, true)
            ArrayList<Project> projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
            ArrayList<Experiment> experimentList = utilsService.getExperimentListForProject(openProject, session?.showInactive ?: false)
            respond projectList, model:[project: openProject, projectCount: projectList.size(), experimentList: experimentList]
        }
    }

    def toggleShowinctive(){
        println "this.checked = ${params.checked}"
        session?.showInactive = !session?.showInactive ?: false
        redirect action: 'list'
    }

//    @Secured(['ROLE_Admin','ROLE_User','ROLE_NewUser','ROLE_Guest','IS_AUTHENTICATED_ANONYMOUSLY', 'ROLE_ANONYMOUS','permitAll'])
//    @Secured(['IS_AUTHENTICATED_ANONYMOUSLY'])
    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)

        //Redirect user to create page if there is no project to be listed
        if (projectList) {
            session.removeAttribute('firstTime')
            respond projectList, model: [projectCount: projectList.size()]
        } else {
            session.firstTime = true;
            redirect action: 'create'
        }
    }

    def create() {
        User user = springSecurityService.currentUser
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
        Project project = new Project(params)
        if(session.firstTime) {
            flash.message = "Please create a project to start using FlowGate.";
            flash.firstTime = "Please create a project to start using FlowGate.";
        }
        respond project
    }

    @Transactional
    def save(Project project) {
        User owner = springSecurityService.currentUser
        if (project == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (project.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            def projectList = utilsService.getProjectListForUser(owner, params, session?.showInactive ?: false)
            respond project.errors, view:'create', model: [projectList: projectList, projectCount: projectList.size(), experimentList:[] ]
            return
        }
        project.save flush:true
        ProjectUser.create(project, owner, 'owner')
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'project.label', default: 'Project'), "${project.title.take(10)+'... '}"])
                redirect action: 'index', params: [pId: project?.id]
            }
            '*' { respond project, [status: CREATED] }
        }
    }

    def edit(Project project) {
        User user = springSecurityService.currentUser
        if(grails.plugin.springsecurity.SpringSecurityUtils.ifNotGranted('ROLE_Administrator,ROLE_Admin') ){
            def projList = ProjectUser.findAllByUser(user)*.project
            if (!(projList.contains(project))) {
//                TODO first and active
                project = projList.first()
            }
        }
        def projectList = utilsService.getProjectListForUser(user, params, session?.showInactive ?: false)
        respond project, model: [projectList: projectList]
    }

    @Transactional
    def update(Project project) {
        if (project == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (project.hasErrors()) {
            //transactionStatus.setRollbackOnly()
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
            //transactionStatus.setRollbackOnly()
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
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        ProjectUser.where { project == project }.deleteAll()
        for(Experiment exp : project.experiments) {
            ExperimentMetadataCategory.where { experiment == exp }.deleteAll()
            ExperimentMetadata.where { experiment == exp }.deleteAll()
            Analysis.where { experiment == exp }.deleteAll()
            Dataset.where { experiment == exp }.deleteAll()
            ExpFile.where { experiment == exp }.deleteAll()
            ExperimentUser.where { experiment == exp }.deleteAll()
            exp.delete()
        }
        project.delete()

        flash.message = message(code: 'default.erased.message', args: [message(code: 'project.label', default: 'Project'), project.title])
        redirect view: 'list'
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
