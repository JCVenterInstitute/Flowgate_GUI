package flowgate

//import grails.converters.JSON
import grails.transaction.Transactional
import grails.web.servlet.mvc.GrailsParameterMap
import org.genepattern.client.GPClient
//import org.genepattern.io.IOUtil
//import org.genepattern.webservice.JobResult
import org.genepattern.webservice.Parameter
import org.grails.web.util.WebUtils

@Transactional
class GenePatternService {

    def springSecurityService
    def grailsApplication

    def getSession(){
        WebUtils.retrieveGrailsWebRequest().getCurrentRequest().session
    }

    def getGpClient(String gpServer, String gpUser, String gpUserPw){
        if(gpUserPw == ''){
            return new GPClient(gpServer, gpUser)
        }
        else{
            return new GPClient(gpServer, gpUser, gpUserPw)
        }
    }

    def getJobNo(Module module,  Parameter[] gpParameters){ //submit to server
        GPClient gpClient = getGpClient(module.server.url, module.server.userName, module.server.userPw)
        Integer jobNumber
        String eMsg = ""
        try {
            jobNumber = gpClient.runAnalysisNoWait(module.name, gpParameters)
        }
        catch (all) {
            jobNumber = -1
            eMsg = all?.rootCause ? all.rootCause : all
        }
        return [jobNo: jobNumber, eMsg: eMsg]
    }

    def getGPServInfo(String gpServ) {
        AnalysisServer aS = AnalysisServer.findByName(gpServ)
        [gpServer: aS.url, gpUser: aS.userName, gpUserPw: aS.userPw]
    }

    Parameter[] setModParams(Module module, GrailsParameterMap params){
        Parameter[] parameters = []
        module.moduleParams.each {
            parameters += new Parameter(it.pKey, params["mp-${it.id}"].toString())
        }
        parameters
    }

}
