package flowgate

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Transactional(readOnly = true)
class AnalysisPipelineController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE"]

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond AnalysisPipeline.list(params), model:[analysisPipelineCount: AnalysisPipeline.count()]
    }

    def show(AnalysisPipeline analysisPipeline) {
        respond analysisPipeline
    }

    def create() {
        Experiment experiment = Experiment.get(params.eId.toLong())
//        params.experiment = experiment
//        params.experiment = Experiment.get(params.eId.toLong())
//        params.remove('eId')
        AnalysisPipeline analysisPipeline = new AnalysisPipeline(experiment: experiment)
        respond analysisPipeline
    }

    @Transactional
    def save(AnalysisPipeline analysisPipeline) {
        if (analysisPipeline == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisPipeline.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond analysisPipeline.errors, view:'create'
            return
        }

        analysisPipeline.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'analysisPipeline.label', default: 'AnalysisPipeline'), analysisPipeline.id])
                redirect analysisPipeline
            }
            '*' { respond analysisPipeline, [status: CREATED] }
        }
    }

    def edit(AnalysisPipeline analysisPipeline) {
        respond analysisPipeline
    }

    @Transactional
    def update(AnalysisPipeline analysisPipeline) {
        if (analysisPipeline == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisPipeline.hasErrors()) {
            transactionStatus.setRollbackOnly()
            respond analysisPipeline.errors, view:'edit'
            return
        }

        analysisPipeline.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'analysisPipeline.label', default: 'AnalysisPipeline'), analysisPipeline.id])
                redirect analysisPipeline
            }
            '*'{ respond analysisPipeline, [status: OK] }
        }
    }

    @Transactional
    def delete(AnalysisPipeline analysisPipeline) {

        if (analysisPipeline == null) {
            transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        analysisPipeline.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysisPipeline.label', default: 'AnalysisPipeline'), analysisPipeline.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'analysisPipeline.label', default: 'AnalysisPipeline'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
