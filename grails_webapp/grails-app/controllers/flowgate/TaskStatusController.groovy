package flowgate

import grails.converters.JSON
//import grails.plugin.springsecurity.annotation.Secured

//@Secured(['ROLE_Admin','ROLE_User'])
//@Secured(['IS_AUTHENTICATED_FULLY'])
class TaskStatusController {

    static allowedMethods = [setStatus: "GET"]

    def wsService

    def setStatus() {

        if(params?.jobId){

            if(params?.jobId?.toInteger() > 0){
                Analysis analysis = Analysis.findByJobNumber(params?.jobId?.toInteger())
                if(analysis){

                       Boolean completed = params?.status == 'Finished' ?: false
//                       Boolean completed = restUtilsService.isComplete(analysis)
//                      if(completed){
                    analysis.analysisStatus = params?.jobId?.toInteger() > 0 ? completed ? 3 : 2 : params?.jobId?.toInteger()
                    analysis.save flush: true
                    wsService.tcMsg(params?.jobId?.toString())
//                      }
                }
            }
            println "got status! ${params?.jobId} ${params?.status}"
            render (msg: "got status! jobId=${params?.jobId} jobStatus=${params?.status}") as JSON
        }
        else{
            render (msg: "got no status!") as JSON
        }
//        redirect controller: 'project', action: 'index'
    }
}
