package flowgate

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Transactional(readOnly = true)
class ModuleParamController {

    static allowedMethods = [save: "POST", update: "PUT"]

    def restUtilsService
    def utilsService

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond ModuleParam.list(params), model:[moduleParamCount: ModuleParam.count()]
    }

    def show(ModuleParam moduleParam) {
        respond moduleParam
    }

    def create() {
        respond new ModuleParam(params)
    }

    @Transactional
    def save(ModuleParam moduleParam) {
        if (moduleParam == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (moduleParam.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond moduleParam.errors, view:'create'
            return
        }

        moduleParam.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'moduleParam.label', default: 'ModuleParam'), moduleParam.pKey])
                redirect moduleParam
            }
            '*' { respond moduleParam, [status: CREATED] }
        }
    }

    def edit(ModuleParam moduleParam) {
        respond moduleParam
    }

    @Transactional
    def update(ModuleParam moduleParam) {
        if (moduleParam == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (moduleParam.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond moduleParam.errors, view:'edit'
            return
        }

        moduleParam.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'moduleParam.label', default: 'ModuleParam'), moduleParam.pKey])
                redirect moduleParam
            }
            '*'{ respond moduleParam, [status: OK] }
        }
    }

    @Transactional
    def delete(ModuleParam moduleParam) {

        if (moduleParam == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        moduleParam.delete flush:true

        flash.message = message(code: 'default.deleted.message', args: [message(code: 'moduleParam.label', default: 'ModuleParam'), moduleParam.pKey])
        redirect controller: 'module', action: 'edit', params: [id: moduleParam?.module?.id]
    }

    @Transactional
    def importParameters(Module module) {
        try {
            def moduleParamsJson = restUtilsService.fetchModuleParamsForModule(module)
            def datasetParam = params.dataset

            List<ModuleParam> moduleParams = utilsService.createModuleParamsFromJson(moduleParamsJson)

            ModuleParam.where{ module == module}.deleteAll()
            for(ModuleParam p : moduleParams) {
                p.module = module
                if(p.pKey.equalsIgnoreCase(datasetParam)) p.pType = 'ds'
                p.save(flush: true, failOnError:true)
            }

            flash.message = "Module parameters are imported for " + module.title
        } catch (Exception e) {
            println e.localizedMessage
            flash.error = e.localizedMessage
        }
        redirect controller: 'module', action: 'edit', params: [id: module?.id]
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'moduleParam.label', default: 'ModuleParam'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }

    def fetchModuleParamsForModule(Module module) {
        try {
            def moduleParamsList = restUtilsService.fetchModuleParamsForModule(module)

            render (contentType:"text/json") {
                success true
                modules "${g.render(template: '/module/moduleParamsModalTemplate', model: [moduleParams: moduleParamsList, module: module])}"
            }
        } catch (Exception e) {
            render (contentType:"text/json") {
                success false
                message e.localizedMessage
            }
        }
    }
}
