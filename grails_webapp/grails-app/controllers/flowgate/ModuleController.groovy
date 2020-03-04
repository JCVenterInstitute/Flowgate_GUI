package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Secured(["ROLE_Administrator","ROLE_Admin"])
@Transactional(readOnly = true)
class ModuleController {

    static allowedMethods = [save: "POST", update: "PUT"]

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond Module.list(params), model:[moduleCount: Module.count()]
    }

    /*
    def show(Module module) {
        respond module
    }
    */

    def create() {
        respond new Module(params)
    }

    @Transactional
    def save(Module module) {
        if (module == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (module.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond module.errors, view:'create'
            return
        }

        module.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'module.label', default: 'Module'), module.title])
                redirect module
            }
            '*' { respond module, [status: CREATED] }
        }
    }

    def edit(Module module) {
        respond module
    }

    @Transactional
    def update(Module module) {
        if (module == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (module.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond module.errors, view:'edit'
            return
        }

        module.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'module.label', default: 'Module'), module.title])
                redirect module
            }
            '*'{ respond module, [status: OK] }
        }
    }

    @Transactional
    def delete(Module module) {

        if (module == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        module.delete flush:true

        flash.message = "Module " + module.title + " deleted!"
        redirect action: 'index'
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'module.label', default: 'Module'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
