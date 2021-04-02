package flowgate

import com.github.jmchilton.blend4j.galaxy.beans.Workflow
import com.google.gson.Gson
import flowgate.transform.GMLInfo
import grails.plugins.rest.client.RestBuilder
import grails.plugins.rest.client.RestResponse
import grails.transaction.Transactional
import grails.web.servlet.mvc.GrailsParameterMap
//import java.net.URLEncoder
//import grails.async.*
//import org.springframework.util.LinkedMultiValueMap
//import org.springframework.util.MultiValueMap
//import static groovyx.net.http.ContentType.BINARY
import groovy.json.JsonOutput
import groovy.json.JsonSlurper
import org.grails.web.util.WebUtils

import java.util.concurrent.TimeUnit

//import grails.async.*
//import java.io.File


//import static grails.async.Promises.*

@Transactional
class RestUtilsService {

  def springSecurityService
  def utilsService
  def fcsService
  def grailsApplication

  def getSession() {
    WebUtils.retrieveGrailsWebRequest().getCurrentRequest().session
  }

  def getJobNo(Module module, ArrayList paramVars) { //submit to server
    def jobInfo
    Integer jobNumber = -1
    String eMsg = ""
    try {
      jobInfo = submitJob(module, paramVars)
      if (jobInfo.status >= 200 && jobInfo.status < 300) {
        jobNumber = jobInfo.jobId.toInteger()
//        TODO catch if setcallback does not work
        def cbOk= setCallback(module, jobNumber)
      }
    }
    catch (all) {
      eMsg = all.toString()
    }
    return [jobNo: jobNumber, eMsg: eMsg]
  }


  def submitJob(Module module, ArrayList paramVars) {
    String lsidOrTaskName = module.name
    if (!lsidOrTaskName.startsWith('urn')) {
      lsidOrTaskName = getUrn(module.server, lsidOrTaskName)?.lsid
    }
    if (lsidOrTaskName != '') {
      RestBuilder rest = new RestBuilder()
      String d_url = module.server.url + "/gp/rest/v1/jobs"
      RestResponse resp = rest.post(module.server.url + "/gp/rest/v1/jobs") {
        contentType "application/json"
        auth "Basic ${utilsService.authEncoded(module.server.userName, module.server.userPw)}"
        json JsonOutput.toJson(['lsid': lsidOrTaskName, 'params': paramVars])
      }
      return ['status': resp.responseEntity.statusCode.value()] << resp.json
    } else return ['status': 405, 'msg': 'E: task not found!']
  }

  def setCallback(Module module, Integer jobId) {
    String callbackUrl = grailsApplication.config.callbackUrl
    String callbackEncoded = URLEncoder.encode(callbackUrl, "UTF-8")
    String bodyStr = "notificationUrl=${callbackEncoded}"
    RestBuilder rest = new RestBuilder()
    RestResponse resp = rest.post(module.server.url + "/gp/rest/v1/jobs/${jobId.toString()}/setNotificationCallback") {
      contentType "application/x-www-form-urlencoded"
      accept "application/json"
      auth "Basic ${utilsService.authEncoded(module.server.userName, module.server.userPw)}"
      body bodyStr
    }
    return ['status': resp.responseEntity.statusCode.value(), 'respJson': resp.json]
  }


  def dwnLdZip(Module module, Integer jobId){
    RestBuilder rest = new RestBuilder()
    RestResponse resp = rest.get(module.server.url + "/gp/rest/v1/jobs/${jobId.toString()}/download") {
      contentType "application/zip"
      auth "Basic ${utilsService.authEncoded(module.server.userName, module.server.userPw)}"
    }
    return ['status': resp.responseEntity.statusCode.value(), 'respBody': resp.responseEntity.body]
  }

  def getUrn(AnalysisServer server, String taskName) {
    RestBuilder rest = new RestBuilder()
    String serverApiUrl = server.url + "/gp/rest/v1/tasks/${taskName}"
    RestResponse resp = rest.get(serverApiUrl) {
      contentType "application/json"
      auth "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
    }
//        TODO check status
    return resp.json
  }

  def getAnnotationHeader(ds) {
    //TODO sort fields by display order!!!
    def fieldsLst = ds.expFiles*.metaDatas*.mdKey
    return fieldsLst.flatten().unique()
  }

  def getAnnotationHeaderStr(ds, separator) {
    //TODO sort fields by display order!!!
    String returnStr = getAnnotationHeader(ds).join(separator)
    return returnStr
  }

  def getAnnotationBody(Dataset ds, String separator) {
    //TODO sort fields by display order!!!
    String bodyStr = ""
    def fields = getAnnotationHeader(ds)
    ds.expFiles.each { expFile ->
      def lineFields = []
      fields.each { metaDataKey ->
        lineFields << expFile.metaDatas.find { it.mdKey == metaDataKey }?.mdVal
      }

      bodyStr += lineFields.join(separator) + "\n"
    }
    return bodyStr
  }

  ArrayList setModParamsRest(Module module, GrailsParameterMap params, def request, boolean isGmlSelected, def experiment) {
// TODO compare result vs module parameters; result is empty on upload errors!
    ArrayList paramVars = []
    if (isGmlSelected) {
      def dsParam = module.moduleParams.find { it.pType.equals('ds')}
      def dsId = params["mp-${dsParam.id}-ds"]

      Dataset ds = Dataset.get(dsId.toLong())
      if (ds) {
        def fcsFilePath;

        ds.expFiles.each { expFile ->
          File fileToUpload = new File(expFile.filePath + expFile.fileName)
          def fileLocation = uploadFileOrDirParams(module, fileToUpload, expFile.fileName)
          paramVars.push(['name': dsParam.pKey, 'values': fileLocation])

          if(!fcsFilePath) {
            fcsFilePath = expFile.filePath + expFile.fileName
          }
        }

        def dirLocation = grailsApplication.config.getProperty('clinical.dir', String)
        def foldershareScriptPath = grailsApplication.config.getProperty('clinical.foldershare', String)
        def phpLocation = grailsApplication.config.getProperty('clinical.php', String)
        def inclusionFile = grailsApplication.config.getProperty('clinical.inclusion', String)
        def exclusionFile = grailsApplication.config.getProperty('clinical.exclusion', String)
        def headerFile = grailsApplication.config.getProperty('clinical.header', String)
        def headerMapFile = grailsApplication.config.getProperty('clinical.headerMap', String)

        String gmlFileOption = params?.gmlFileOption

        File gmlFile;
        if(gmlFileOption.equals("clinical")) {
          File dir = new File(dirLocation)
          def drupalServer = AnalysisServer.findByPlatform(3);
          def gmlFileName = params?.gmlFile
          def downloadCommand = "$phpLocation $foldershareScriptPath --host $drupalServer.url --username $drupalServer.userName --password $drupalServer.userPw download \"/$experiment.project.id/$experiment.id/$gmlFileName\""

          Process process = Runtime.getRuntime().exec(downloadCommand, null, dir)
          def out = new StringBuffer()
          def err = new StringBuffer()
          process.consumeProcessOutput(out, err)
          process.waitFor(10, TimeUnit.SECONDS)

          if (out.size() > 0) {
            println "LS Project Folder Output: $out"
            out.setLength(0);
          }

          if (err.size() > 0) {
            println "LS Project Folder Error: $err"
            err.setLength(0);
          }

          gmlFile = new File(dir.getPath() + File.separator + gmlFileName)
        } else {
          def partFile = request.getFile("gmFileUpload");
          gmlFile = partFile.part.fileItem.tempFile
        }

        def info = new GMLInfo(gmlFile)
        def fcsFile = new File(fcsFilePath)
        fcsService.readFile(fcsFile, false)

        def inclusionFilePath = dirLocation + File.separator + inclusionFile
        def headerMapFilePath = dirLocation + File.separator + headerMapFile
        def headerFilePath = dirLocation + File.separator + headerFile
        def exclusionFilePath = dirLocation + File.separator + exclusionFile

        def fosInclusion = new FileWriter(inclusionFilePath)
        def dosInclusion = new PrintWriter(fosInclusion)
        def fosHeaderMapping = new FileWriter(headerMapFilePath)
        def dosHeaderMapping = new PrintWriter(fosHeaderMapping)

        def gateId = 1;
        def gateNames = []
        for (gate in info.gates) {
          def sb = new StringBuilder()
          sb.append(gateId) //Pop_ID

          def name = gate.key
          def value = gate.value

          def gateName = "";
          def index = 0;
          for (child in value.children) {
            if(child.markerLabel) {
              gateName += child.markerLabel + "x"
              gateNames.push(child.markerLabel)
              dosHeaderMapping.println("$child.markerName\t$child.markerLabel")
            } else {
              def editedMarkerName = child.markerName.replaceAll(" ", "_")
              gateName += editedMarkerName + "x"

              gateNames.push(editedMarkerName)
              dosHeaderMapping.println("$child.markerName\t$editedMarkerName")
            }

            def markerIndex = fcsService.channelShortname.findIndexOf { it.equals(child.markerName)}
            def dimensionIndex = markerIndex + 1
            if (index == 0) {
              sb.append("\t$dimensionIndex") //DimensionX
            } else {
              //sb.insert((int) (Math.log10(gateId) + 1) + 2, "\t$dimensionIndex") //DimensionY (get number of digits in gateId to find correct offset)
              def indexOfDimensionX = sb.indexOf("\t", 2)
              sb.insert(indexOfDimensionX, "\t$dimensionIndex") //DimensionY
            }

            def channelRange = fcsService.channelRange[markerIndex]
            def ampValue = fcsService.ampValue[markerIndex]

            if(child.transformationRef != null) {
              def transformation = info.transforms[child.transformationRef]

              def max = transformation.transformInternal(channelRange)
              def min = transformation.transformInternal(ampValue)

              //for linear transformation in FCSTrans, the range will be normalized to 0-1023
              if(max < 1023)
                max = 1023

              def ratio = 200 / max
              def dafiMax = (int) (ratio * child.maxByDimension)
              def dafiMin = (int) (ratio * child.minByDimension)

              if(dafiMin < 0)
                dafiMin = 0

              sb.append("\t$dafiMin\t$dafiMax") //MinX or MaxY
            } else {
              def max = channelRange
              def min = ampValue

              //for linear transformation in FCSTrans, the range will be normalized to 0-1023
              if(max < 1023)
                max = 1023

              def ratio = 200 / max
              def dafiMax = (int) (ratio * child.maxByDimension)
              def dafiMin = (int) (ratio * child.minByDimension)

              if(dafiMin < 0)
                dafiMin = 0

              sb.append("\t$dafiMin\t$dafiMax") //MinX or MaxY
            }

            index++
          }

          //Parent_ID, 0 if gateId = 1
          if(gateId != 1 && value.parentId) {
            def parentGate = info.gates[value.parentId]
            sb.append("\t$parentGate.order")
          } else {
            sb.append("\t0")
          }
          sb.append("\t0") //Cluster_Type
          sb.append("\t0") //Visualize_or_Not
          sb.append("\t1") //Recluster_or_Not
          sb.append("\t").append(gateName.substring(0, gateName.length() - 1)) //Cell_Phenotype
          sb.append("\n") //End of line

          dosInclusion.print(sb.toString())
          gateId++
        }
        dosInclusion.close()
        fosInclusion.close()
        dosHeaderMapping.close()
        fosHeaderMapping.close()

        def fosHeader = new FileWriter(headerFilePath)
        def dosHeader = new PrintWriter(fosHeader)
        dosHeader.print(gateNames.join(","))
        dosHeader.close()
        fosHeader.close()

        def fosExclusion = new FileWriter(exclusionFilePath)
        def dosExclusion = new PrintWriter(fosExclusion)
        dosExclusion.print("1\t0\t0\t0\t0\t0\t0\t0\t0\t0")
        dosExclusion.close()
        fosExclusion.close()

        gmlFile.delete()

        def inc = new File(inclusionFilePath)
        def incLocation = uploadFileOrDirParams(module, inc, inclusionFile)
        paramVars.push(['name': 'config.file', 'values': incLocation])
        inc.delete()

        def exc = new File(exclusionFilePath)
        def excLocation = uploadFileOrDirParams(module, exc, exclusionFile)
        paramVars.push(['name': 'rev.config.file', 'values': excLocation])
        exc.delete()

        def hMap = new File(headerMapFilePath)
        def hMapLocation = uploadFileOrDirParams(module, hMap, headerMapFile)
        paramVars.push(['name': 'header_replace.txt', 'values': hMapLocation])
        hMap.delete()

        def h = new File(headerFilePath)
        def hLocation = uploadFileOrDirParams(module, h, headerFile)
        paramVars.push(['name': 'header.list', 'values': hLocation])
        h.delete()
      } else {
        println "E: no Dataset!"
      }
    } else {
      module.moduleParams.each {
        switch (it.pType) {
          case 'ds':
            def dsId = params["mp-${it.id}-ds"]
            Dataset ds = Dataset.get(dsId.toLong())
            if (ds) {
              ds.expFiles.each { expFile ->
                File fileToUpload = new File(expFile.filePath + expFile.fileName)
                def fileLocation = uploadFileOrDirParams(module, fileToUpload, expFile.fileName)
                paramVars.push(['name': it.pKey, 'values': fileLocation])
              }
            } else {
              println "E: no Dataset!"
            }
            break

          case 'dir':
            def dir = request.getFiles("mp-${it.id}")
            dir.each { dirFile ->
              if (!dirFile.filename.contains('/.')) {
                String filename = dirFile.filename.replaceAll(/^.*\//, "")
                def fileLocation = uploadFileOrDirParams(module, dirFile.part.fileItem.tempFile, filename)
                paramVars.push(['name': it.pKey, 'values': fileLocation])
              }
            }
            break

          case 'file':
            def partFile = request.getFile("mp-${it.id}")
            if (!partFile.filename.startsWith('.')) {
              def fileLocation = uploadFileOrDirParams(module, partFile.part.fileItem.tempFile, partFile.filename)
              paramVars.push(['name': it.pKey, 'values': fileLocation])
            }
            break

          case 'meta': //upload metadata.txt
            String tabSep = "\t"
            def dsParamId = params["mp-meta"]
            def dsId = params["mp-${dsParamId}-ds"]

            Dataset ds = Dataset.get(dsId.toLong())
            String metaDataFilePrefix = 'metadata'
            String metaDataFileSuffix = '.txt'
            File metaDataFile = File.createTempFile(metaDataFilePrefix, metaDataFileSuffix)
            //TODO sort fields by display order!!!
            metaDataFile.write(getAnnotationHeaderStr(ds, '\t') + '\n' + getAnnotationBody(ds, '\t'))
            def fileLocation = uploadFileOrDirParams(module, metaDataFile, metaDataFilePrefix + metaDataFileSuffix)
            metaDataFile.delete()
            paramVars.push(['name': it.pKey, 'values': fileLocation])
            break

          case 'field': //upload description.txt
            String descrFilePrefix = 'description'
            String descrFileSuffix = '.txt'
            File descrFile = File.createTempFile(descrFilePrefix, descrFileSuffix)
            descrFile.write(params.analysisName + System.lineSeparator() + params.analysisDescription)
            def dsParamId = params["mp-meta"]
            def dsId = params["mp-${dsParamId}-ds"]
            if (dsId != null) {
              Dataset ds = Dataset.get(dsId.toLong())
              descrFile.append(System.lineSeparator() + ds.name)
            }
            def fileLocation = uploadFileOrDirParams(module, descrFile, descrFilePrefix + descrFileSuffix)
            descrFile.delete()
            paramVars.push(['name': it.pKey, 'values': fileLocation])
            break

          default:
            paramVars.push(['name': it.pKey, 'values': [params["mp-${it.id}"].toString()]])
            break
        }
      }
    }
    paramVars
  }


  def uploadFileOrDirParams(Module module, File fileOrDirPath, String fName) {
    def ulResult = [:]
    def ulFiles = []
    if (fileOrDirPath.isFile()) {
      ulResult = doUploadFile(module.server, fileOrDirPath, fName)
      if (!(ulResult.code == 200)) {
        println "Error uploading file!"
        return
      }
      ulFiles.add(ulResult?.location)
    }
    ulFiles
  }

  def doUploadFile(AnalysisServer server, File pathAndFile, String fName) {
    if (!server.url || server.url == "") {
      log.error "Error no server url!"
      return [code: 500, location: '']
    }
    //    TODO put gp data upload url in config
    String uploadApiPath = server.url + "/gp/rest/v1/data/upload/job_input?name=${fName.replaceAll(" ", "_")}"
    RestBuilder rest = new RestBuilder()
    RestResponse resp
    String authStr = "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
    try {
      resp = rest.post(uploadApiPath) {
        auth authStr
        contentType "application/octet-stream"
        body pathAndFile.bytes
      }
    }
    catch (all) {
      println all?.message
      return [code: 408, location: '']
    }
    if (!(resp?.responseEntity?.statusCodeValue >= 201 && resp?.responseEntity?.statusCodeValue < 300)) {
      log.error "Error uploading file! ${resp?.responseEntity?.statusCodeValue.toString()}"
      return [code: resp?.responseEntity?.statusCodeValue, location: '']
    } else {
      return [code: 200, location: resp.responseEntity.body]
    }
  }

  def jobResult(Analysis analysis) {
    jobResult(analysis, false)
  }

  def jobResult(Analysis analysis, boolean onlyStatus) {
    RestBuilder rest = new RestBuilder()
    RestResponse resp
    String serverApiUrl = analysis.module.server.url + "/gp/rest/v1/jobs/${analysis.jobNumber}" + (onlyStatus ? "/status.json" : "")
    try {
      resp = rest.get(serverApiUrl) {
        contentType "application/json"
        auth "Basic ${utilsService.authEncoded(analysis.module.server.userName, analysis.module.server.userPw)}"
      }
    }
    catch (all) {
      println all?.message
    }
    log.debug('jobResult status ' + resp?.responseEntity?.statusCodeValue?.toString())
    ['statusCode': resp.responseEntity.statusCode] << resp.json
  }

  Boolean isComplete(Analysis analysis) {
    jobResult(analysis)?.status?.isFinished
  }

  Boolean isPending(Analysis analysis) {
    jobResult(analysis, true).isPending
  }

  def fetchModulesForServer(AnalysisServer server) {
    RestBuilder rest = new RestBuilder()
    RestResponse resp
    JsonSlurper jsonSlurper = new JsonSlurper()
    try {
      resp = connectToGenePatternServer(rest, server)

      if (resp.statusCodeValue == 401)
        throw new Exception("Authentication failed! Please check server username and password.")

      if (resp.statusCodeValue == 200) {
        String allTasksUrl = server.url + "/gp/rest/v1/tasks/all.json"

        resp = rest.get(allTasksUrl) {
          contentType "application/json"
          auth "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
        }

        def respBody = jsonSlurper.parseText(resp.responseEntity.body)

        return respBody.all_modules
      } else {
        //Otherwise try ImmportGalaxy
        GalaxyService galaxyService = new GalaxyService(server)
        List<Workflow> workflows = galaxyService.fetchImmportGalaxyWorkflows();

        return jsonSlurper.parseText(new Gson().toJson(workflows))
      }
    }
    catch (all) {
      throw new Exception(all.getCause() ? all.getCause().getMessage() : all.getMessage())
    }
  }

  def fetchModuleParamsForModule(Module module) {
    RestBuilder rest = new RestBuilder()
    RestResponse resp
    JsonSlurper jsonSlurper = new JsonSlurper()
    def server = module.server
    try {
      resp = connectToGenePatternServer(rest, server)

      if (resp.statusCodeValue == 401)
        throw new Exception("Authentication failed! Please check server username and password.")

      if (resp.statusCodeValue == 200) {
        String moduleUrl = server.url + "/gp/rest/v1/tasks/" + module.name + "?includeEula=false&includeProperties=false"

        resp = rest.get(moduleUrl) {
          contentType "application/json"
          auth "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
        }

        def respBody = jsonSlurper.parseText(resp.responseEntity.body)

        def attributes = respBody.params.collect {
          it.collect { key, value ->
            return value.get("attributes")
          }
        }

        return attributes.flatten()
      }
    }
    catch (all) {
      throw new Exception(all.getCause() ? all.getCause().getMessage() : all.getMessage())
    }
  }

  def connectToGenePatternServer(RestBuilder rest, AnalysisServer server) {
    String gpWadlUrl = server.url + "/gp/rest/application.wadl"
    //check if GenePattern server
    RestResponse resp = rest.get(gpWadlUrl) {
      contentType "application/json"
      auth "Basic ${utilsService.authEncoded(server.userName, server.userPw)}"
    }

    return resp;
  }
}
