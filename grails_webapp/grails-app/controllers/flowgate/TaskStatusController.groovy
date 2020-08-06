package flowgate

import grails.converters.JSON
import grails.plugins.rest.client.RestBuilder
import grails.plugins.rest.client.RestResponse

//import grails.plugin.springsecurity.annotation.Secured

//@Secured(['ROLE_Admin','ROLE_User'])
//@Secured(['IS_AUTHENTICATED_FULLY'])
class TaskStatusController {

    static allowedMethods = [setStatus: ["GET","POST"]]

    def wsService

    def setStatus() {

        if(params?.jobId){

            if(params?.jobId?.toInteger() > 0){
                Analysis analysis = Analysis.findByJobNumber(params?.jobId?.toInteger())
                if(analysis){
                    switch (params?.status){
                        case 'Finished':
                            RestBuilder rest = new RestBuilder()
                            RestResponse resp
                            String serverApiUrl = analysis.module.server.url + "/gp/rest/v1/jobs/${analysis.jobNumber}/status.json"
                            try {
                                resp = rest.get(serverApiUrl) {
                                    contentType "application/json"
                                    auth "Basic ${utilsService.authEncoded(analysis.module.server.userName, analysis.module.server.userPw)}"
                                }
                                println "GenePattern job ${analysis.jobNumber} status: ${resp.json}"
                            }
                            catch (all) {
                                println all?.message
                            }
                            analysis.analysisStatus = 3

                            if(resp.json && resp.json.completedInGp) {
                                analysis.dateCompleted = resp.json && resp.json.completedInGp
                            }
                            break
                        case 'Processing': analysis.analysisStatus = 2
                            break
                        case 'Error': analysis.analysisStatus = -1
                            break
                        default: analysis.analysisStatus = 2
                            break
                    }
                    analysis.save flush: true
                    wsService.tcMsg(params?.jobId?.toString())
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
