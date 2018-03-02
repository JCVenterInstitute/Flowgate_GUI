package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import org.springframework.transaction.TransactionStatus

@Secured(['ROLE_Administrator','ROLE_Admin'])
class AsyncProjectController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE"]

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        Project.async.task {
            [projectList: list(params), count: count() ]
        }.then { result ->
            respond result.projectList, model:[projectCount: result.count]
        }
    }

    def show(Long id) {
        Project.async.get(id).then { project ->
            respond project
        }
    }

    def create() {
        respond new Project(params)
    }

    def save(Project project) {
        Project.async.withTransaction { TransactionStatus status ->
            if (project == null) {
                status.setRollbackOnly()
                notFound()
                return
            }

            if(project.hasErrors()) {
                status.setRollbackOnly()
                respond project.errors, view:'create' // STATUS CODE 422
                return
            }

            project.save flush:true
            request.withFormat {
                form multipartForm {
                    flash.message = message(code: 'default.created.message', args: [message(code: 'project.label', default: 'Project'), project.id])
                    redirect project
                }
                '*' { respond project, [status: CREATED] }
            }
        }
    }

    def edit(Long id) {
        Project.async.get(id).then { project ->
            respond project
        }
    }

    def update(Long id) {
        Project.async.withTransaction { TransactionStatus status ->
            def project = Project.get(id)
            if (project == null) {
                status.setRollbackOnly()
                notFound()
                return
            }

            project.properties = params
            if( !project.save(flush:true) ) {
                status.setRollbackOnly()
                respond project.errors, view:'edit' // STATUS CODE 422
                return
            }

            request.withFormat {
                form multipartForm {
                    flash.message = message(code: 'default.updated.message', args: [message(code: 'Project.label', default: 'Project'), project.id])
                    redirect project
                }
                '*'{ respond project, [status: OK] }
            }
        }
    }

    def delete(Long id) {
        Project.async.withTransaction { TransactionStatus status ->
            def project = Project.get(id)
            if (project == null) {
                status.setRollbackOnly()
                notFound()
                return
            }

            project.delete flush:true

            request.withFormat {
                form multipartForm {
                    flash.message = message(code: 'default.deleted.message', args: [message(code: 'Project.label', default: 'Project'), project.id])
                    redirect action:"index", method:"GET"
                }
                '*'{ render status: NO_CONTENT }
            }
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