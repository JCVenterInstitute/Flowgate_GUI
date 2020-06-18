package flowgate

import grails.plugin.springsecurity.annotation.Secured

import static org.springframework.http.HttpStatus.*
import grails.transaction.Transactional


@Secured(["ROLE_Administrator","ROLE_Admin"])
@Transactional
class AnalysisServerController {

    def springSecurityService

    static allowedMethods = [save: "POST", update: "PUT"]

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

        analysisServer.user = springSecurityService.currentUser
        if (analysisServer.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond analysisServer.errors, view:'create', model:[analysisServer: analysisServer]
            return
        }

        analysisServer.save flush:true

        flash.message = "Server " + analysisServer.name + " created!"

        redirect action: 'index'
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
        flash.message = "Server " + analysisServer.name + " has been updated!"
        redirect action: 'edit', params: [id: analysisServer.id]
    }

    @Transactional
    def delete(AnalysisServer analysisServer) {

        if (analysisServer == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        analysisServer.delete flush:true

        flash.message = "Server deleted!"
        redirect action: 'index'
    }

    def updatePassword(AnalysisServer server) {
        String oldPass = params?.oldpass
        String newPass = params?.newpass

//        if(!oldPass.equals(server.userPw)) {
//            flash.passError = "Old Password does not match!"
//        } else {
            server.userPw = newPass
            server.save flush:true

//            flash.passSuccess = "Password has been updated!"
            flash.success = "Password has been updated!"
//        redirect action: 'edit', params: [id: server.id]
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
