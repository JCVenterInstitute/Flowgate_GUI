package flowgate

import com.sun.jersey.api.client.ClientHandlerException
import grails.converters.JSON
import grails.core.GrailsApplication
import grails.plugin.springsecurity.annotation.Secured
import grails.transaction.Transactional
import grails.web.http.HttpHeaders
import org.apache.commons.io.IOUtils
import org.genepattern.webservice.Parameter
import org.grails.core.io.ResourceLocator

import java.util.zip.ZipOutputStream
import java.util.zip.ZipEntry
import java.awt.image.BufferedImage
import javax.imageio.ImageIO

import static org.springframework.http.HttpStatus.*

//@Transactional(readOnly = true)
//@Secured(['isAuthenticated()'])
@Secured(["IS_AUTHENTICATED_FULLY"])
class AnalysisController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE", axSelectAllFcs: "GET", axUnselectAllFcs: "GET", d3data: "GET",  del: ["DELETE","GET"]]

    def springSecurityService
    def genePatternService
    def restUtilsService
    def utilsService
    def scheduledTaskService

    GrailsApplication grailsApplication


    def axShowResultsModal(){
        render(contentType: 'text/json') {
            success true
            modalData "${g.render(template:'templates/resultModal', model: [jobNumber: params?.jobId, id: params?.id]) }"
        }
    }

    def axSelectAllFcs(){
        render(contentType: 'text/json') {
            success true
            contentList "${g.render(template:'templates/expFilesSelect', model: [eId: params?.eId, expFileSelected: ExpFile.findAllByExperimentAndIsActive(Experiment.get(params?.eId?.toLong()), true)*.id])}"
        }
    }

    def axUnselectAllFcs(){
        render(contentType: 'text/json') {
            success true
            contentList "${g.render(template:'templates/expFilesSelect', model: [eId: params?.eId, expFileSelected: [] ])}"
        }
    }

    def axModulChange(){
        if(params.modId) {
            Module module = Module.get(params.modId.toLong())
            if(params.dsId)
                (module.moduleParams.find { it -> it.pType.equals("ds")}).defaultVal = params.dsId
            render(contentType: 'text/json') {
                success true
                modParams "${g.render(template: 'templates/moduleParams', model: [module: module])}"
            }
        }
        else{
            render(contentType: 'text/json'){
                success false
            }
        }
    }

    def index(Integer max) {
        Experiment experiment = Experiment.findById(params.eId)
        scheduledTaskService.jobList = utilsService.getUnfinishedJobsListOfUser(experiment)
        params.max = Math.min(max ?: 10, 100)
        def analysisList = utilsService.getAnalysisListByUser(experiment, params)
        respond analysisList, model: [analysisCount: utilsService.getAnalysisListByUser(experiment, null).size(), eId: params?.eId, experiment: experiment]
    }

    /*
    // deprecated: used to update the table row when status gets changed
    def renderJobRow() {
        Integer jobNo = params?.jobNo?.toInteger()
        Analysis bean = Analysis.findByJobNumber(jobNo)
        println "in renderJobRow ${jobNo} bn_status: ${bean.analysisStatus}"
        render(contentType:"text/json", [success: true, tablRow: "${g.render( template: 'templates/analysisListTablRow', model: [bean: bean] )}" ] as JSON)
    }
    */

    def show(Analysis analysis) {
        respond analysis
    }

    def showResults(Analysis analysis) {
        def jobResult
        if(!analysis.isFailedOnSubmit()){
            try {
                jobResult = restUtilsService.jobResult(analysis)
            }
            catch (all){
                println 'Error! No job result (maybe deleted on server)!' + all.dump()
            }
        }
        if(jobResult.statusCode.value != 200 && jobResult.statusCode.value != 201){
            String dummy = jobResult?.statusCode?.toString()
            flash.resultMsg = jobResult?.statusCode?.toString() + " - " + jobResult?.statusCode?.reasonPhrase
        }
        respond analysis, model: [jobResult: jobResult]
    }


    def displayResults(Analysis analysis) {
        def jobResult
        if(!analysis.isFailedOnSubmit()){
            try {
                jobResult = restUtilsService.jobResult(analysis)
            }
            catch (all){
                println 'Error! No job result (maybe deleted on server)!' + all.dump()
            }
        }
        if(jobResult.statusCode.value != 200 && jobResult.statusCode.value != 201){
            String dummy = jobResult?.statusCode?.toString()
            flash.resultMsg = jobResult?.statusCode?.toString() + " - " + jobResult?.statusCode?.reasonPhrase
        }
        respond analysis, model: [jobResult: jobResult]
    }

    def dwnldZip(){
        String zipFileName = "file.zip"
        String inputDir = "logs"

        ZipOutputStream zipFile = new ZipOutputStream(new FileOutputStream(zipFileName))
        new File(inputDir).eachFile() { file ->
            zipFile.putNextEntry(new ZipEntry(file.getName()))
            def buffer = new byte[1024]
            file.withInputStream { i ->
                def l = i.read(buffer)
//              check wether the file is empty
                if (l > 0) {
                    zipFile.write(buffer, 0, l)
                }
            }
            zipFile.closeEntry()
        }
        zipFile.close()

//        response.setHeader("Content-disposition", "filename=\"${album.title}.zip\"")
//        response.contentType = "application/zip"
//        response.outputStream << baos.toByteArray()
//        response.outputStream.flush()
    }

    def dwnldZip2(){
        /*
        ByteArrayOutputStream baos = new ByteArrayOutputStream()
        ZipOutputStream zipFile = new ZipOutputStream(baos)

        album.tracks.each {track ->
            if (track.mp3DownloadPath != "") {
                File file = new File(grailsApplication.config.tracks.root.directory+track.mp3DownloadPath)
                zipFile.putNextEntry(new ZipEntry(track.title+".mp3"))
                file.withInputStream { i ->

                    zipFile << i

                }
                zipFile.closeEntry()
            }
        }
        zipFile.finish()
        response.setHeader("Content-disposition", "filename=\"${album.title}.zip\"")
        response.contentType = "application/zip"
        response.outputStream << baos.toByteArray()
        response.outputStream.flush()
        */
    }

    /*
     File myFile = new File('/Users/acs/Sources/flowgate/grails-app/views/analysis/results/UCSD_CLL_New.html')
//        render file: myFile, contentType: 'text/html'
        Analysis analysis = Analysis.get(params?.analysisId)
        def outputFile = JSON.parse(params.outputFile)
        def fileUrl = new URL(outputFile.link.href)
        def connection = fileUrl.openConnection()
        connection.setRequestProperty ("Authorization", utilsService.authHeader(analysis.module.server.userName,analysis.module.server.userPw))
        def dataStream = connection.inputStream
        if(params.download != null && params.download){
            response.setContentType("application/octet-stream")
            response.setHeader("Content-disposition", "Attachment; filename=${outputFile.path}")
        } else {
            response.setContentType("text/html")
        }
        response.outputStream << dataStream
        response.outputStream.flush()
//        //String dwnLdMsg = "File '${params?.filename}' successfully downloaded!"
    */

    def downloadFile() {
        Analysis analysis = Analysis.get(params?.analysisId)
        def outputFile = JSON.parse(params.outputFile)
        def fileUrl = new URL(outputFile.link.href)
        def connection = fileUrl.openConnection()
        connection.setRequestProperty ("Authorization", utilsService.authHeader(analysis.module.server.userName,analysis.module.server.userPw))
        def dataStream = connection.inputStream
        if(params.download != null && params.download){
            response.setContentType("application/octet-stream")
            response.setHeader("Content-disposition", "Attachment; filename=${outputFile.path}")
        } else {
            response.setContentType("text/html")
        }
        response.outputStream << dataStream
        response.outputStream.flush()
//        //String dwnLdMsg = "File '${params?.filename}' successfully downloaded!"
    }

    def resultFile(String analysisId, String filename) {
        Analysis analysis = Analysis.get(analysisId)
        File resultFile = utilsService.getResultFileDetails(filename, analysis, null)
        response.setContentType("text/tab-separated-values")
        response.outputStream << (InputStream) new FileInputStream(resultFile)
        response.outputStream.flush()
    }

    def resultFileWithJobId(String analysisId, String jobId, String filename) {
        Analysis analysis = Analysis.get(analysisId)
        File resultFile = utilsService.getResultFileDetails(filename, analysis, jobId)
        response.setContentType("text/tab-separated-values")
        response.outputStream << (InputStream) new FileInputStream(resultFile)
        response.outputStream.flush()
    }

    def downloadResultReport() {
        /*
        params.download .... switch to either download the file or open in browser
        download=true .... downloads file
        download=null or false .... opens file in browser
         */
        def dataStream
        def jobResult
        Analysis analysis = Analysis.get(params?.analysisId)
        if (analysis.module.server.isImmportGalaxyServer()) {
            String resultFileStoragePath = grailsApplication.config.getProperty('resultFileStoreLocation.path', String)
            Optional<String> resultOpt = Arrays.stream(analysis.renderResult.split(",")).findFirst();

            if (resultOpt.isPresent()) {
                File resultFile = new File(resultFileStoragePath + File.separator + resultOpt.get())
                if (params.download != null && params.download) {
                    response.setContentType("application/octet-stream")
                    response.setHeader("Content-disposition", "Attachment; filename=${resultFile.name}")
                } else {
                    response.setContentType("text/html")
                }
                response.outputStream << (InputStream) new FileInputStream(resultFile)
                response.outputStream.flush()
            }
        } else {
            if (!analysis.isFailedOnSubmit()) {
                try {
                    jobResult = restUtilsService.jobResult(analysis)
                }
                catch (all) {
                    println 'Error! No job result (maybe deleted on server)!' + all.dump()
                }
            }
            if (jobResult?.statusCode?.value != 200 && jobResult?.statusCode?.value != 201) {
                String dummy = jobResult?.statusCode?.toString()
                flash.resultMsg = jobResult?.statusCode?.toString() + " - " + jobResult?.statusCode?.reasonPhrase
                render "<p><strong style='color:red;'>Error:</strong> No result file found to download!</p>"
            } else {
                def outputFile = jobResult.outputFiles.find { it.path == analysis?.renderResult }  // 'Reports/AutoReport.html'
                if (outputFile) {
                    def fileUrl = new URL(outputFile.link.href)
                    def connection = fileUrl.openConnection()
                    connection.setRequestProperty("Authorization", utilsService.authHeader(analysis.module.server.userName, analysis.module.server.userPw))
                    if (connection) {
                        try {
                            dataStream = connection?.inputStream
                        }
                        catch (e) {
                            response.setContentType("text/html")
                            render "<p><strong style='color:red;'>Error:</strong> No result file found to download!</p>"
                            return
                        }
                        if (params.download != null && params.download) {
                            response.setContentType("application/octet-stream")
                            response.setHeader("Content-disposition", "Attachment; filename=${outputFile.path}")
                        } else {
                            response.setContentType("text/html")
                        }
                        if (dataStream) {
                            response.outputStream << dataStream
                            response.outputStream.flush()
                        } else {
                            response.setContentType("text/html")
                            render "<p><strong style='color:red;'>Error:</strong> No result file found to download!</p>"
                        }
                    } else {
                        render status: 401, "<p><strong style='color:red;'>Error:</strong> No result file found to download!</p>"
                    }
                }
            }
        }
    }

    def downloadErrorFile() {
        def jobResult
        Analysis analysis = Analysis.get(params?.analysisId)
        if(!analysis.isFailedOnSubmit()){
            try {
                jobResult = restUtilsService.jobResult(analysis)
            }
            catch (all){
                println 'Error! No job result (maybe deleted on server)!' + all.dump()
                render "<p><strong style='color:red;'>Error:</strong> No job found! plesae check if server is alive and job exists.</p>"
            }
        }
        if(jobResult.statusCode.value != 200 && jobResult.statusCode.value != 201){
            flash.resultMsg = jobResult?.statusCode?.toString() + " - " + jobResult?.statusCode?.reasonPhrase
            render"<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
        } else {
            def outputFile = jobResult.outputFiles.find { it.path == analysis?.renderResult }
            // 'Reports/AutoReport.html'
            if (outputFile) {
                def fileUrl = new URL(outputFile.link.href)
                def connection = fileUrl.openConnection()
                connection.setRequestProperty("Authorization", utilsService.authHeader(analysis.module.server.userName, analysis.module.server.userPw))
                if (connection) {
                    try {
                        dataStream = connection?.inputStream
                    }
                    catch (e) {
                        response.setContentType("text/html")
                        render "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
                        //                    flash.message = "Error: No result file found!"
                        //                    render controller: 'analysis', view: 'index'
                        //                    redirect controller: 'analysis', action: 'index'
                        return
                    }
                    if (params.download != null && params.download) {
                        response.setContentType("application/octet-stream")
                        response.setHeader("Content-disposition", "Attachment; filename=${outputFile.path}")
                    } else {
                        response.setContentType("text/html")
                    }
                    if (dataStream) {
                        response.outputStream << dataStream
                        response.outputStream.flush()
                    } else {
                        response.setContentType("text/html")
                        render "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
                    }
                } else {
                    response.setContentType("text/html")
                    render status: 401, "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
                }

            }
        }
        def errorFile = jobResult.status.stderrLocation  // 'Reports/AutoReport.html'
        if(errorFile) {
            def fileUrl = new URL(errorFile)
            def connection = fileUrl.openConnection()
            connection.setRequestProperty("Authorization", utilsService.authHeader(analysis.module.server.userName, analysis.module.server.userPw))
            def dataStream = connection.inputStream
            response.setContentType("text/plain")
            response.outputStream << dataStream
            response.outputStream.flush()
        } else {
            render "<p><strong style='color:red;'>Error:</strong> No error file found!</p>"
        }
    }


    def downloadZipResults() {
        /*
        params.download .... switch to either download the file or open in browser
        download=true .... downloads file
        download=null or false .... opens file in browser
         */
        def dataStream
        def jobResult
        Analysis analysis = Analysis.get(params?.analysisId)
        if(analysis.jobNumber!= -1){
            try {
                jobResult = restUtilsService.jobResult(analysis)
            }
            catch (all){
                println 'Error! No job result (maybe deleted on server)!' + all.dump()
            }
        }
        if(jobResult.statusCode.value != 200 && jobResult.statusCode.value != 201){
            String dummy = jobResult?.statusCode?.toString()
            flash.resultMsg = jobResult?.statusCode?.toString() + " - " + jobResult?.statusCode?.reasonPhrase
            render"<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
        } else {
            def fileUrl = new URL( "${analysis?.module?.server?.url}/gp/rest/v1/jobs/${analysis?.jobNumber}/download")
            def connection = fileUrl.openConnection()
            connection.setRequestProperty("Authorization", utilsService.authHeader(analysis?.module?.server?.userName, analysis?.module?.server?.userPw))
            if (connection) {
                try {
                    dataStream = connection?.inputStream
                }
                catch (e) {
                    response.setContentType("text/html")
                    render "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No result file found to download!</div>"
                    return
                }
                if (params.download != null && params.download) {
                    response.setContentType("application/octet-stream")
                    response.setHeader("Content-disposition", "Attachment; filename=${analysis?.jobNumber}.zip")
                } else {
                    response.setContentType("text/html")
                }
                if (dataStream) {
                    response.outputStream << dataStream
                    response.outputStream.flush()
                } else {
                    response.setContentType("text/html")
                    render "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> File not found!</div>"
                }
            } else {
                response.setContentType("text/html")
                render status: 401, "<div style='font-size:x-large;margin:auto;width:20%;padding-top:20%;'><strong style='color:red;'>Error:</strong> No connection!</div>"
            }
        }
    }

    /*
        def resp = restUtilsService.dwnLdZip(analysis?.module, analysis?.jobNumber)
        if(resp.status==200){
            webRequest.renderView = false
//            String body = resp?.respBody
//            byte[] data = zipContent.getBytes("UTF-8")
//            ByteArrayOutputStream zipStream = new ByteArrayOutputStream()
            def content = resp?.respBody?.getBytes()
            def zipStream = new ByteArrayInputStream(content)
//            zipStream.write(resp?.respBody?.bytes)
//            response.setHeader(HttpHeaders.CONTENT_DISPOSITION, "attachment; ${analysis?.jobNumber}.zip")
            response.setHeader("Content-dsiposition", "inline; filename=${analysis?.jobNumber}.zip")
            response.setContentType("application/zip")
//            response.setContentType("file-mime-type")
            response.setCharacterEncoding("UTF-8")
            response.contentLength = resp?.respBody?.bytes?.size()
//            response.setContentLength (data.length)
//            response.outputStream << zipStream.toByteArray()
//            response.outputStream << resp?.respBody?.bytes?.toByteArray()
            IOUtils.copy(zipStream, response.outputStream)
//            response.outputStream << data
            response.outputStream.flush()
            response.outputStream.close()
//            byte[] data = resp?.respBody?.getBytes("UTF-8")
//            return
//            IOUtils.closeQuietly(resp.respBody.bytes)
//            render file: resp?.respBody?.bytes("UTF-8"), filename: "${analysis?.jobNumber}.zip", contentType: "APPLICATION/OCTET-STREAM"
//            render file: resp?.respBody?.bytes, filename: "${analysis?.jobNumber}.zip", contentType: "application/zip"
         */




    def getImage(){
        def path = params.filepath
        //returns an image to display
        BufferedImage originalImage = ImageIO.read(new File(path));
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        def fileext = path.substring(path.indexOf(".")+1, path.length())

        ImageIO.write( originalImage, fileext, baos );
        baos.flush();

        byte[] img = baos.toByteArray();
        baos.close();
        response.setHeader('Content-length', img.length.toString())
        response.contentType = "image/"+fileext // or the appropriate image content type
        response.outputStream << img
        response.outputStream.flush()
    }


    def create() {
        Analysis analysis = new Analysis(params)
        Experiment experiment = Experiment.findById(params.eId)
        def dsList = Dataset.findAllByExperiment(experiment)
        respond analysis, model: [eId: params.eId, experiment: experiment, dsCount: dsList.size()]
    }

    /*
    def checkStatus(){
        def jobsList = JSON.parse(params?.jobs)
        Boolean pCheckNeeded = utilsService.periodicCheckNeeded(jobsList)
        if(pCheckNeeded){
            jobsList.each {
                Integer jobNumber = it.toInteger()
                Analysis analysis = Analysis.findByJobNumber(jobNumber)
                if(jobNumber > 0){
                    Boolean completed = restUtilsService.isComplete(analysis)
                    analysis.analysisStatus = jobNumber > 0 ? completed ? 3 : 2 : jobNumber
                    analysis.save flush: true
                }
            }
        }
        render(contentType: 'text/json') {
            status statusMap
        }
    }
    */

    /*
    def checkDbStatus(){ //refresh dataTable
//        TODO move to service
        def currentUser = springSecurityService.currentUser
        def analysisList
        def jobList
        Experiment experiment = Experiment.findById(params.eId)
        if(SpringSecurityUtils.ifAnyGranted("ROLE_Administrator,ROLE_Admin")){
            analysisList = Analysis.findAllByExperiment(experiment, params)
            jobList = Analysis.findAllByExperimentAndAnalysisStatusNotInList(experiment, [3])*.jobNumber
        }
        else {
            analysisList = Analysis.findAllByExperimentAndUserAndAnalysisStatusNotInList(experiment, currentUser, [-2], params)
            jobList = Analysis.findAllByExperimentAndUserAndAnalysisStatusNotInList(experiment,currentUser, [3,-2])*.jobNumber
        }
        String updChkStr = ""
//        TODO clear also if no jobs with positiv jobNo
        if(!utilsService.periodicCheckNeeded(jobList)){
            updChkStr = "clear"
        }
        render(contentType: 'text/json') {
            success true
            updChkStatus updChkStr
            tablTempl "${g.render(template:'templates/analysisListTbl', model: [analysisList: analysisList, analysisCount: analysisList.size(), jobList: jobList])}"
        }
    }
    */

    /*
    @Transactional
    def save(Analysis analysis) {
        def thisRequest = request
        Module module = Module.get(params?.module?.id)
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        Dataset dataset = Dataset.findByExperiment(experiment)
            analysis.experiment = experiment
            if (module.server.isGenePatternServer()) {
                Map resultMap
                if (module.server.url.startsWith('https')) {
                    Parameter[] parameters = genePatternService.setModParams(module, params)
                    resultMap = genePatternService.getJobNo(module, parameters)
                } else {
                    def parameters = restUtilsService.setModParamsRest(module, params, thisRequest)
                    resultMap = restUtilsService.getJobNo(module, parameters)
                }
                analysis.jobNumber = resultMap.jobNo
                flash.eMsg = resultMap.eMsg
                analysis.analysisStatus = resultMap.jobNo > 0 ? 2 : -1 //pending/processing or error
            } else {
                GalaxyService galaxyService = new GalaxyService(module.server)
                analysis.jobNumber = galaxyService.submitImmportGalaxyWorkflow(module, experiment, params, thisRequest)
                analysis.analysisStatus = 2
            }
            experiment.addToAnalyses(analysis)
            analysis.validate()
            if (analysis == null) {
                //transactionStatus.setRollbackOnly()
                notFound()
                return
            }
            if (analysis.hasErrors()) {
                //transactionStatus.setRollbackOnly()
                respond analysis.errors, view: 'create', model: [eId: params.eId, experiment: experiment]
                return
            }
//        try {
            analysis.save flush: true
            AnalysisDataset.create analysis, dataset
//        } catch (ClientHandlerException e) {
//            flash.error = "Server " + e.cause.message + " is not available."
//            redirect action: 'create', model: [analysis: analysis], params: [eId: params?.eId]
//            return
//        }
            request.withFormat {
                form multipartForm {
                    flash.message = message(code: 'default.created.message', args: [message(code: 'analysis.label', default: 'Analysis:'), analysis.analysisName])
//                redirect controller: 'experiment', action: 'index', params: [eId: params?.eId]
                    redirect action: 'index', params: [eId: params?.eId]
                }
                '*' { respond analysis, [status: CREATED] }
            }
    }
    */

    @Transactional
    def save(Analysis analysis) {
        def thisRequest = request
        Module module = Module.get(params?.module?.id)
        Experiment experiment = Experiment.get(params?.eId?.toLong())
        String mpDatasetId = module.moduleParams.find {it.pKey == 'Input.Dir'}.id
        Dataset dataset = Dataset.get(params["mp-${mpDatasetId}-ds"])
        analysis.experiment = experiment
        if (module.server.isGenePatternServer()) {
            Map resultMap
            if (module.server.url.startsWith('https')) {
                Parameter[] parameters = genePatternService.setModParams(module, params)
                resultMap = genePatternService.getJobNo(module, parameters)
            } else {
                def parameters = restUtilsService.setModParamsRest(module, params, thisRequest)
                resultMap = restUtilsService.getJobNo(module, parameters)
            }
            analysis.jobNumber = resultMap.jobNo
            flash.eMsg = resultMap.eMsg
            analysis.analysisStatus = resultMap.jobNo > 0 ? 2 : -1 //pending/processing or error
        } else {
            GalaxyService galaxyService = new GalaxyService(module.server)
            analysis.jobNumber = galaxyService.submitImmportGalaxyWorkflow(module, experiment, params, thisRequest)
            analysis.analysisStatus = 2
        }
        experiment.addToAnalyses(analysis)
        analysis.dataset = dataset
        analysis.validate()
        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
        if (analysis.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond analysis.errors, view: 'create', model: [eId: params.eId, experiment: experiment]
            return
        }
//        try {
        analysis.save flush: true
//        AnalysesDatasets.create analysis, dataset
//        } catch (ClientHandlerException e) {
//            flash.error = "Server " + e.cause.message + " is not available."
//            redirect action: 'create', model: [analysis: analysis], params: [eId: params?.eId]
//            return
//        }
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.created.message', args: [message(code: 'analysis.label', default: 'Analysis:'), analysis.analysisName])
//                redirect controller: 'experiment', action: 'index', params: [eId: params?.eId]
                redirect action: 'index', params: [eId: params?.eId]
            }
            '*' { respond analysis, [status: CREATED] }
        }
    }


    def edit(Analysis analysis) {
        respond analysis
    }

    @Transactional
    def update(Analysis analysis) {
        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
//        analysis.experiment = Experiment.get(params?.experiment)
//        analysis.user = User.get(params?.user)
        analysis.validate()

        if (analysis.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            analysis.experiment = Experiment.get(params?.experimentId)
            analysis.experiment.project.attach()
            analysis.user.attach()
            respond analysis, view:'edit'
            return
        }

        analysis.save flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
//                redirect analysis
                redirect controller: 'experiment', action: 'index', params: [eId: analysis?.experiment?.id]
            }
            '*'{ respond analysis, [status: OK] }
        }
    }

    @Transactional
    def delete(Analysis analysis) {

        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }

        analysis.delete flush:true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action:"index", method:"GET", params: [eId: analysis?.experiment?.id]
            }
            '*'{ render status: NO_CONTENT }
        }
    }

    @Transactional
    def del(Analysis analysis) {
//        TODO check if delete on server should be done as well
        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
//        analysis.delete flush:true
        analysis.analysisStatus = -2
        analysis.save flush:true

        request.withFormat {
            /*
            DOES NOT WORK!
            html {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action:"index", method:"GET", params: [eId: analysis?.experiment?.id]
            }
            */
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action:"index", method:"GET", params: [eId: analysis?.experiment?.id]
            }
            '*'{
//                render status: NO_CONTENT
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action:"index", method:"GET", params: [eId: analysis?.experiment?.id]
            }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'analysis.label', default: 'Analysis'), params.id])
                redirect action: "index", method: "GET"
            }
            '*'{ render status: NOT_FOUND }
        }
    }
}
