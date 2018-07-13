package flowgate

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional

@Transactional(readOnly = true)
class AnalysisServerController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE"]

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        respond AnalysisServer.list(params), model:[analysisServerCount: AnalysisServer.count()]
    }

    def show(AnalysisServer analysisServer) {
        respond analysisServer
    }

    def create() {
        respond new AnalysisServer(params)
    }

    @Transactional
    def save(AnalysisServer analysisServer) {
        if (analysisServer == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisServer.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond analysisServer.errors, view:'create'
            return
        }

        analysisServer.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'analysisServer.label', default: 'AnalysisServer'), analysisServer.id])
                redirect analysisServer
            }
            '*' { respond analysisServer, [status: CREATED] }
        }
    }

    def edit(AnalysisServer analysisServer) {
        respond analysisServer
    }

    @Transactional
    def update(AnalysisServer analysisServer) {
        if (analysisServer == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        if (analysisServer.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond analysisServer.errors, view:'edit'
            return
        }

        analysisServer.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'analysisServer.label', default: 'AnalysisServer'), analysisServer.id])
                redirect analysisServer
            }
            '*'{ respond analysisServer, [status: OK] }
        }
    }

    @Transactional
    def delete(AnalysisServer analysisServer) {

        if (analysisServer == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        analysisServer.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysisServer.label', default: 'AnalysisServer'), analysisServer.id])
                redirect action:"index", method:"GET"
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'analysisServer.label', default: 'AnalysisServer'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
