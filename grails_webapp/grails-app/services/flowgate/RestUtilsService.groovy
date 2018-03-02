package flowgate

import grails.http.client.*
import grails.plugins.rest.client.RestBuilder

//import grails.artefact.controller.RestResponder
//import grails.converters.JSON
//import com.budjb.httprequests.HttpClientFactory
//import com.budjb.httprequests.filter.bundled.BasicAuthFilter
import grails.plugins.rest.client.RestResponse
import grails.transaction.Transactional
import grails.web.servlet.mvc.GrailsParameterMap
import groovy.io.FileType
import groovy.json.JsonOutput
import org.apache.commons.codec.binary.Base64
import org.grails.web.util.WebUtils

//import grails.async.*
import static groovyx.net.http.ContentType.BINARY
import java.io.File
//import grails.async.*
//import org.json.JSONObject
//import static groovyx.net.http.ContentType.URLENC
//import static groovyx.net.http.ContentType.JSON
@Transactional
class RestUtilsService {

    def springSecurityService
    def utilsService

    def getSession(){
        WebUtils.retrieveGrailsWebRequest().getCurrentRequest().session
    }

    def getJobNo(Module module, ArrayList paramVars){ //submit to server
        def jobInfo
        Integer jobNumber = -1
        String eMsg = ""
        try {
            jobInfo = submitJob(module, paramVars)
            if(jobInfo.status >= 200 && jobInfo.status < 300 ){
                jobNumber = jobInfo.jobId.toInteger()
            }
        }
        catch (all) {
//            eMsg = all?.rootCause ? all.rootCause : all
            eMsg = all.toString()
        }
        return [jobNo: jobNumber, eMsg: eMsg]
    }

    def submitJob(Module module, ArrayList paramVars){
        String lsidOrTaskName = module.name
        if(!lsidOrTaskName.startsWith('urn')){
            lsidOrTaskName = getUrn(module.server, lsidOrTaskName)?.lsid
        }
        if(lsidOrTaskName!=''){
            RestBuilder rest = new RestBuilder()
            RestResponse resp = rest.post(module.server.url + "/gp/rest/v1/jobs") {
                contentType "application/json"
                auth "Basic ${utilsService.authEncoded(module.server.userName, module.server.userPw)}"
                json JsonOutput.toJson(['lsid': lsidOrTaskName, 'params': paramVars])
            }
            return ['status': resp.responseEntity.statusCode.value()] << resp.json
        }
        else return ['status': 405, 'msg': 'E: task not found!']
    }

    def getUrn(AnalysisServer server, String taskName){
        RestBuilder rest = new RestBuilder()
        String serverApiUrl = server.url + "/gp/rest/v1/tasks/${taskName}"
        RestResponse resp = rest.get( serverApiUrl ) {
            contentType "application/json"
            auth "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
        }
//        TODO check status
        return resp.json
    }

    ArrayList setModParamsRest(Module module, GrailsParameterMap params, def request) {
        // TODO compare result vs module parameters; result is empty on upload errors!
        ArrayList paramVars = []
        module.moduleParams.each {
            switch(it.pType){
                case ['dir']: def dir = request.getFiles("mp-${it.id}")
                              dir.each{ dirFile ->
                                 if(!dirFile.filename.contains('/.')){
                                    String filename = dirFile.filename.replaceAll(/^.*\// , "" )
                                    def fileLocation = uploadFileOrDirParams(module, dirFile.part.fileItem.tempFile, filename )
                                    paramVars.push( ['name': it.pKey, 'values': fileLocation] )
                                 }
                             }
                    break

                case ['file']: def partFile = request.getFile("mp-${it.id}")
                               if(!partFile.filename.startsWith('.')) {
                                   def fileLocation = uploadFileOrDirParams(module, partFile.part.fileItem.tempFile, partFile.filename)
                                   paramVars.push(['name': it.pKey, 'values': fileLocation])
                               }
                    break

                default: paramVars.push( ['name': it.pKey, 'values': [params["mp-${it.id}"].toString()]] )
                    break
            }
        }
        paramVars
    }

    def uploadFileOrDirParams(Module module, File fileOrDirPath, String fName) {
        def ulResult = [:]
        def ulFiles = []
        if(fileOrDirPath.isFile()){
            ulResult = doUploadFile(module.server, fileOrDirPath, fName)
            if (!(ulResult.code == 200)) {
                println "Error uploading file!"
                return
            }
            ulFiles.add( ulResult.location )
        }
        ulFiles
    }

    def doUploadFile(AnalysisServer server, File pathAndFile, String fName){
        String uploadApiPath = server.url + "/gp/rest/v1/data/upload/job_input?name=${fName}"
        def response = [:]
        withRest(uri: uploadApiPath, ignoreSSLIssues: true) {
            def resp = post(
                    headers: [ Authorization: utilsService.authHeader(server.userName,server.userPw)],
                    body: pathAndFile.bytes,
                    requestContentType: BINARY
            )
            if(!(resp.responseBase.h.original.code >= 201 && resp.responseBase.h.original.code < 300)){
                println "Error uploading file!"
                response = [code: resp.responseBase.h.original.code, location: '']
            }
            else {
                response=[code: 200, location: resp.responseData.str]
            }
        }
        response
    }

    def jobResult(Analysis analysis){
        RestBuilder rest = new RestBuilder()
        RestResponse resp
        String serverApiUrl = analysis.module.server.url + "/gp/rest/v1/jobs/${analysis.jobNumber.toString()}"
        try {
            resp = rest.get ( serverApiUrl ){
                contentType "application/json"
                auth "Basic ${utilsService.authEncoded(analysis.module.server.userName, analysis.module.server.userPw)}"
            }
        }
        catch (all) {
            println all?.message
        }
        log.debug('jobResult status ' + resp?.responseEntity?.statusCodeValue?.toString() )
        ['statusCode': resp.responseEntity.statusCode] << resp.json
    }

    Boolean isComplete(Analysis analysis){
        Boolean dummy = jobResult(analysis).status.isFinished
        dummy
    }

    Boolean isPending(Analysis analysis){
        jobResult(analysis).status.isPending
    }

}
