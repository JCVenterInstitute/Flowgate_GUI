package flowgate

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Transactional(readOnly = true)
class AnalysisPipelineElementController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE"]

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond AnalysisPipelineElement.list(params), model:[analysisPipelineElementCount: AnalysisPipelineElement.count()]
    }

    def show(AnalysisPipelineElement analysisPipelineElement) {
        respond analysisPipelineElement
    }

    def create() {
        respond new AnalysisPipelineElement(params)
    }

    @Transactional
    def save(AnalysisPipelineElement analysisPipelineElement) {
        if (analysisPipelineElement == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisPipelineElement.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond analysisPipelineElement.errors, view:'create'
            return
        }

        analysisPipelineElement.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'analysisPipelineElement.label', default: 'AnalysisPipelineElement'), analysisPipelineElement.id])
                redirect analysisPipelineElement
            }
            '*' { respond analysisPipelineElement, [status: CREATED] }
        }
    }

    def edit(AnalysisPipelineElement analysisPipelineElement) {
        respond analysisPipelineElement
    }

    @Transactional
    def update(AnalysisPipelineElement analysisPipelineElement) {
        if (analysisPipelineElement == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisPipelineElement.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond analysisPipelineElement.errors, view:'edit'
            return
        }

        analysisPipelineElement.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'analysisPipelineElement.label', default: 'AnalysisPipelineElement'), analysisPipelineElement.id])
                redirect analysisPipelineElement
            }
            '*'{ respond analysisPipelineElement, [status: OK] }
        }
    }

    @Transactional
    def delete(AnalysisPipelineElement analysisPipelineElement) {

        if (analysisPipelineElement == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        analysisPipelineElement.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysisPipelineElement.label', default: 'AnalysisPipelineElement'), analysisPipelineElement.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'analysisPipelineElement.label', default: 'AnalysisPipelineElement'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
