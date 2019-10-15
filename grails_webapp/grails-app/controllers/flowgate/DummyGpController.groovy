package flowgate

import grails.converters.JSON
import grails.plugin.springsecurity.annotation.Secured

@Secured(['IS_AUTHENTICATED_ANONYMOUSLY','ROLE_Administrator','ROLE_Admin'])
class DummyGpController {

    def index() { }


    def jobResult(Analysis analysis){
        String serverResponse = "isFinished"
        Map tStatus = ['isFinished': false, "isPending": true]
        println "----------------------------- called / checking ----------------------------------"
        println "params ${params}"
        if(params.jobId=='36'){
          tStatus = ['isFinished': true, "isPending": false]
        }
        render (contentType: 'application/json', [success: false, responseStatus: serverResponse, status: tStatus] as JSON)
    }
}
