package flowgate

import grails.converters.JSON
import grails.core.GrailsApplication
import grails.plugin.springsecurity.SpringSecurityUtils
import grails.transaction.Transactional
import grails.web.servlet.mvc.GrailsParameterMap
//import org.apache.commons.codec.binary.Base64
import org.grails.web.util.WebUtils

import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths
import java.util.stream.Collectors
import java.util.stream.Stream
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

@Transactional
class UtilsService {

    def springSecurityService
    def restUtilsService
    def wsService

    GrailsApplication grailsApplication

    String overviewFilePath = "http://immportgalaxy-int.jcvi.org:8080/static/flowtools/js/overview.js"

    def getSession(){
        WebUtils.retrieveGrailsWebRequest().getCurrentRequest().session
    }

    def getOpenStatus(String object, Long id){
        def session = getSession()
        switch(object) {
            case 'project':
                return session.projectOpenId == id
                break
            case 'experiment':
                return session.experimentOpenId == id
                break
            case 'expFile':
                return session.expFileOpenId == id
                break
        }
    }

    def setAppInitVars(){
        def session = getSession()
        UserSettings userSettings = UserSettings.findByUser(springSecurityService.currentUser)
        if(session && userSettings){
            session.projectOpenId = userSettings.projectOpenId ?: 0
            session.projectEditModeId = userSettings.projectEditModeId ?: 0
            session.experimentOpenId = userSettings.experimentOpenId ?: 0
            session.experimentEditModeId = userSettings.experimentEditModeId ?: 0
            List idsLst = [JSON.parse(userSettings.expFileOpenIds) ?: 0]
            session.expFileOpenIds = idsLst
        }
        else println "Error: no session!!!!"
    }

    def isOwnerMember (String object, Long objectId, String role){
        if (object){
            switch (object){
                case 'project':  return (ProjectUser.findAllByProjectAndUser(Project.get(objectId), springSecurityService.currentUser )*.projRole).contains(role)
                    break
                case 'experiment':  return (ExperimentUser.findAllByExperimentAndUser(Experiment.get(objectId), springSecurityService.currentUser )*.expRole).contains(role)
                    break
            }
        }
        return false
    }

    def isAffil(String object, Long objectId){
        return isOwnerMember(object, objectId, 'owner') || isOwnerMember(object, objectId, 'member')
    }

    def clone(String cloneType, def source, def parent, Boolean createChilds, String appendix){
        Boolean saveok = false
        switch (cloneType){
            case 'project': Project projectDest = new Project(source.properties)
                projectDest.title += appendix
                projectDest.experiments = []
                saveok = projectDest.save flush: true
                if(createChilds)
                    source?.experiments?.each {
                        clone('experiment', it, projectDest, true, '')
                    }
                break


            case 'experiment': Experiment experimentDest = new Experiment(source.properties)
                experimentDest.title += appendix
                experimentDest.project = parent
                experimentDest.expFiles = []
                experimentDest.experimentMeta = []
                experimentDest.experimentHypothesis = []
                saveok = experimentDest.save flush: true
                if(createChilds)
                    source.expFiles.sort{ it.id }.each{
                        clone('expFile', it, experimentDest, true, '')
                    }

                break

            case 'expFile': ExpFile expFileDest = new ExpFile(source.properties)
                expFileDest.title += appendix
                expFileDest.experiment = parent
                saveok = expFileDest.save flush: true
                break
        }
        return saveok
    }

    def doManage(Experiment experiment, List<Long> cmpLst, String role){
        List<Long> srcLst = ExperimentUser.findAllByExperimentAndExpRole(experiment, role)*.user.id
        if(cmpLst == null) cmpLst = new ArrayList<>(0);
        ((srcLst - cmpLst) + (cmpLst - srcLst)).each{
            if(springSecurityService.currentUser.id != it.toLong()) { //you cannot remove yourself!
                User experimentUser = User.get(it.toLong())
                if (ExperimentUser.findAllByExperimentAndUserAndExpRole(experiment, experimentUser, role).size() == 0)
                    addExperimentUser(experiment, experimentUser, role)
                else
                    removeExperimentUser(experiment, experimentUser, role)

            }
        }
    }

    def addExperimentUser(Experiment experiment, User user, String userType){
        ExperimentUser.create(experiment, user, userType)
    }

    def removeExperimentUser(Experiment experiment, User user, String userType){
        ExperimentUser.where{ experiment == experiment && user == user && expRole == userType }.deleteAll()
    }

    def doManage(Project project, List<Long> cmpLst, String role){
        List<Long> srcLst = ProjectUser.findAllByProjectAndProjRole(project, role)*.user.id
        ((srcLst - cmpLst) + (cmpLst - srcLst)).each{
            if(springSecurityService.currentUser.id != it.toLong()) { //you cannot remove yourself!
                User projectUser = User.get(it.toLong())
                if (ProjectUser.findAllByProjectAndUserAndProjRole(project, projectUser, role).size() == 0)
                    addProjectUser(project, projectUser, role)
                else
                    removeProjectUser(project, projectUser, role)

            }
        }
    }

    def addProjectUser(Project project, User user, String userType){
        ProjectUser.create(project, user, userType)
    }

    def removeProjectUser(Project project, User user, String userType){
        ProjectUser.where{ project == project && user == user && projRole == userType }.deleteAll()
    }

    String authEncoded(String username, String password){
//        return new String(Base64.encodeBase64((username+':'+password).getBytes()), "UTF-8")
      return "${username}:${password}".encodeAsBase64()
    }

    String authHeader(String username, String password){
        return "Basic "+authEncoded(username, password)
    }


    def addIdToSession(Long id, String sVar){
        def session = getSession()
        if(session[sVar]) {
            println "session[ ${sVar} ] == ${session[sVar]}"
            if (!session[sVar]?.toList().contains(id)) {
                println "addin to ${sVar} ${id}"
                session[sVar].add(id)
            }
        }
        else {
            println "init ${sVar} with ${id}"
            session[sVar] = [id]
        }
    }

    def getProjectListForUser(User user, Map paginateParams, Boolean showInactive) {

        if(user.username.equals("admin"))
            if(showInactive)
                return Project.findAll([params: paginateParams])
            else
                return Project.findAllByIsActive(true, [params: paginateParams])
        else {
            def projectUserList = ProjectUser.findAllByUser(user, [params: paginateParams])
            List<Project> projectList = new ArrayList<Project>(projectUserList.size())
            for (def projectUser : projectUserList) {
                projectList.add(projectUser.project)
            }

            return projectList
        }
    }

    List<Experiment> getExperimentListForProject(Project project, Boolean showInactive){
        if(showInactive){
            return Experiment.findAllByProject(project)
        }
        else {
            return Experiment.findAllByProjectAndIsActive(project, true)
        }
    }

    def csvMetadataParse(experiment, fileListMap, headers){
        println fileListMap
        println "${fileListMap["SID"].unique()}"
        Integer keyOrder = 1
        String fcsFileMatchColumn = "FCS File Name"
//        TODO read from config file
//        String category = 'Basics'
//        ExperimentMetadataCategory category = ExperimentMetadataCategory.findOrCreateByExperimentAndMdCategory(experiment, 'Basics')
        ExperimentMetadataCategory category = ExperimentMetadataCategory.findOrSaveByExperimentAndMdCategory(experiment, 'Basics')
//        TODO handle category
//        if(fileListMap.keySet.contains('category')){
//            category =
//        }
        def metaDataKeys = headers - fcsFileMatchColumn
        metaDataKeys.each{ metadataKey ->
            ExperimentMetadata metaDat = ExperimentMetadata.findByExperimentAndMdCategoryAndMdKey(experiment, category, metadataKey)
            if(!metaDat){
//                metaDat = new ExperimentMetadata(experiment: experiment, mdCategory: category, dispOnFilter: true, visible: true, dispOrder: keyOrder, mdKey: metadataKey).save(flush: true)
                metaDat = new ExperimentMetadata(experiment: experiment, mdCategory: category, dispOnFilter: true, visible: true, dispOrder: keyOrder, mdKey: metadataKey).save()
            }
            else {
                metaDat.properties = [dispOnFilter: true, visible: true, dispOrder: keyOrder, mdKey: metadataKey]
                metaDat.save(flush: true)
            }
            println "${metaDat?.hasErrors()}"
//            if(!metaDat.hasErrors()){}
            Integer valueOrder = 10
            fileListMap["${metadataKey}"].unique().each{ metadataValue ->
                ExperimentMetadataValue metaValue = ExperimentMetadataValue.findByExpMetaDataAndMdValue(metaDat, metadataValue)
                if(!metaValue){
                    metaValue = new ExperimentMetadataValue(expMetaData: metaDat, dispOrder: valueOrder, mdType: 'String', mdValue: metadataValue).save(flush: true)
                }
                else{
                    metaValue.properties = ['dispOrder': valueOrder, 'mdType': 'String']
                    metaValue.save(flush: true)
                }
                println "${metaValue?.hasErrors()}"
                valueOrder += 10
            }
            keyOrder += 1
        }
        doFileAnnotation(experiment, fileListMap, fcsFileMatchColumn, metaDataKeys)
        return
    }

    def fcytMetadataParse(experiment, fileListMap, headers){
//        Integer dispOrderVal = 1
        fileListMap.each { dataRow ->
//            println "importing ${dataRow['Category']} ${dataRow['Key']} ${dataRow['Value']}"
            if(dataRow['Category'] && dataRow['Key']){
                ExperimentMetadataCategory category = ExperimentMetadataCategory.findOrSaveByMdCategoryAndDispOnFilterAndVisible(dataRow["Category"], false, true)
//                Integer dispOrderVal = ExperimentMetadata.findAllByExperimentAndMdCategory(experiment, category).size() * 5 ?: 1
                ExperimentMetadata metaDat = ExperimentMetadata.findOrSaveByExperimentAndMdCategoryAndMdKeyAndVisibleAndIsMiFlowAndDispOnFilter(experiment, category, dataRow['Key'], true, true, false)
                if(dataRow['Value']){
                    ExperimentMetadataValue mDatVal = ExperimentMetadataValue.findOrSaveByExpMetaDataAndMdValueAndMdTypeAndDispOrder(metaDat, dataRow['Value'], 'String', 1)
//                    println "created/added ${mDatVal}"
                }
            }
        }
    }

    def doFileAnnotation(Experiment experiment, annotationMap, fileNameMatchColumn, metaDataKeys){
        annotationMap.each{ lineMap ->
            String searchExpFileName = lineMap["${fileNameMatchColumn}"]
            def expFiles = experiment.expFiles
            ExpFile expFile = expFiles.find{ it.fileName == searchExpFileName }
            if(expFile){
                Integer metaValOrder = 10
                metaDataKeys.each{ metaDataKey ->
                    ExpFileMetadata expFileMetadata = ExpFileMetadata.findByExpFileAndMdKey(expFile, metaDataKey, lineMap["${metaDataKey}"])
                    if(!expFileMetadata){
                        expFileMetadata = new ExpFileMetadata(expFile: expFile, mdKey: metaDataKey, mdVal: lineMap["${metaDataKey}"], dispOrder: metaValOrder).save(flush:true)
                    }
                    else {
                        expFileMetadata.properties =  [dispOrder: metaValOrder]
                        expFileMetadata.save(flush:true)
                    }
                    println "${expFileMetadata.hasErrors()}"
                    metaValOrder += 10
                }
            }

        }
    }

    def getLowestUniqueDispOrder(Experiment experiment) {
        Integer num = 1
        while (num>0) {
            if(experiment.expMetadatas.dispOrder.findAll{it == num}.size()!=0) {
                num++
            }
            else return num
        }
    }

    boolean containsKeyStartsWith(GrailsParameterMap map, String val) {
        for (String key : map.keySet()) {
            if (key.startsWith(val)) {
                return true
            }
        }
        return false
    }

    List<Analysis> getAnalysisListByUser(Experiment experiment, params){
        if(SpringSecurityUtils.ifAnyGranted("ROLE_Administrator,ROLE_Admin")){
            return Analysis.findAllByExperiment(experiment, params)
        }
        else {
            return Analysis.findAllByExperimentAndUserAndAnalysisStatusNotInList(experiment, springSecurityService.currentUser, [-2], params)
        }
    }

    def getUnfinishedJobsListOfUser(Experiment experiment) {
        if(SpringSecurityUtils.ifAnyGranted("ROLE_Administrator,ROLE_Admin")){
            return Analysis.findAllByExperimentAndAnalysisStatusNotInList(experiment, [Analysis.Status.REPORT_FILE_MISSING.value(), Analysis.Status.FINISHED.value(), Analysis.Status.FAILED.value()])*.jobNumber
        }
        else {
            return Analysis.findAllByExperimentAndUserAndAnalysisStatusNotInList(experiment, springSecurityService.currentUser, [Analysis.Status.FINISHED, Analysis.Status.DELETED, Analysis.Status.FAILED])*.jobNumber
        }
    }

    def checkJobStatus(def jobLst) {
        jobLst.each { jobId ->
            Analysis analysis = Analysis.findByJobNumber(jobId)
            if (!analysis.isFailedOnSubmit()) {
                if(analysis.module.server.isGenePatternServer()) {
                    def jobResult = restUtilsService.jobResult(analysis)
                    Boolean completed = jobResult.status?.isFinished;
                    if (completed) {
                        def outputFile = jobResult.outputFiles.find { it.path == analysis?.renderResult }  // 'Reports/AutoReport.html'
                        analysis.analysisStatus = outputFile ? 3 : 4 //add a new status 4 if report file is missing
                        analysis.save flush: true
                        wsService.tcMsg(jobId.toString())
                    }
                } else {
                    GalaxyService galaxyService = new GalaxyService(analysis.module.server)
                    def jobDetails = galaxyService.getInvocationStatus(analysis.module.name, jobId)

                    if(jobDetails.state.equals('error')) {
                        analysis.analysisStatus = -1
                        analysis.save flush: true
                        wsService.tcMsg(jobId.toString())
                    } else if (jobDetails.state.equals('ok')) {
                        File resultFile = galaxyService.downloadFile(jobDetails.outputs.html_file.id)
                        def filePath = saveResultFile(resultFile, analysis)
                        updateDependencies(filePath, analysis.module.server.url, analysis.id)
                        analysis.analysisStatus = 3
                        analysis.renderResult = filePath
                        analysis.save flush: true
                        wsService.tcMsg(jobId.toString())
                    }
                }
            }
        }
    }

    def saveResultFile(def resultFile, def analysis) {
        def htmlFilePath
        String resultFileStoragePath = grailsApplication.config.getProperty('resultFileStoreLocation.path', String)

        String folderPath = resultFileStoragePath + File.separator +
                analysis.experiment.project.id + File.separator +
                analysis.experiment.id + File.separator +
                analysis.id

        File destDir = new File(folderPath)
        byte[] buffer = new byte[1024];
        ZipInputStream zis = new ZipInputStream(new FileInputStream(resultFile))
        ZipEntry zipEntry = zis.getNextEntry();
        while (zipEntry != null) {
            File newFile = newFile(destDir, zipEntry);
            newFile.getParentFile().mkdirs()
            FileOutputStream fos = new FileOutputStream(newFile);
            int len;
            while ((len = zis.read(buffer)) > 0) {
                fos.write(buffer, 0, len);
            }
            fos.close();
            zipEntry = zis.getNextEntry();

            if(newFile.getName().endsWith("html"))
                htmlFilePath = newFile.getCanonicalPath()
        }
        zis.closeEntry();
        zis.close();

        return htmlFilePath
    }

    //To avoid Zip Slip vulnerability which writes files to the file system outside of the target folder
    public static File newFile(File destinationDir, ZipEntry zipEntry) throws IOException {
        File destFile = new File(destinationDir, zipEntry.getName());

        String destDirPath = destinationDir.getCanonicalPath();
        String destFilePath = destFile.getCanonicalPath();

        if (!destFilePath.startsWith(destDirPath + File.separator)) {
            throw new IOException("Entry is outside of the target dir: " + zipEntry.getName());
        }

        return destFile;
    }

    def updateDependencies(def filePath, def serverPath, def analysisId) {
        //Update result file pathes in overview file and embed into the result file
        InputStream is = new URL(overviewFilePath).openStream()
        BufferedReader reader = new BufferedReader(new InputStreamReader(is))
        Stream<String> lines = reader.lines()

        List<String> overview = lines
                .map({ line ->
                    line.replaceAll("./boxplotData.json", analysisId + "/boxPlotData.json")
                            .replaceAll("\\bflow.mfi_pop\\b", analysisId + "/flow.mfi_pop")
                            .replaceAll("\\bflow.mfi\\b", analysisId + "/flow.mfi")
                            .replaceAll("flow.overview", analysisId + "/flow.overview")
                            .replaceAll("flow.sample", analysisId + "/flow.sample")
                })
                .collect(Collectors.toList())
        overview.add(0, "<script>")
        overview.add("</script>")

        Path path = Paths.get(filePath)
        lines = Files.lines(path)
        List<String> replaced = lines
                .map({ line -> line.contains("overview.js") ? String.join("\n", overview) : line.replaceAll("/static", serverPath + "/static") })
                .collect(Collectors.toList())
        Files.write(path, replaced)

        Optional<Path> flowOverviewPathOpt = Files.walk(path.getParent()).filter({ file -> file.getFileName().toString().equals("flow.overview") }).findFirst()
        if (flowOverviewPathOpt.isPresent()) {
            Path flowOverviewPath = flowOverviewPathOpt.get()
            lines = Files.lines(flowOverviewPath)
            replaced = lines
                    .map({ line -> line.replaceAll("<img src=\"", "<img src=\"" + analysisId + "/") })
                    .collect(Collectors.toList())
            Files.write(flowOverviewPath, replaced)
        }
    }

    def getResultFileDetails(def name, def analysis) {
        String resultFileStoragePath = grailsApplication.config.getProperty('resultFileStoreLocation.path', String)

        String filePath = resultFileStoragePath + File.separator +
                analysis.experiment.project.id + File.separator +
                analysis.experiment.id + File.separator +
                analysis.id + File.separator +
                name

        return new File(filePath)
    }

    def createModuleParamsFromJson(def moduleParamsJson) {
        List<ModuleParam> moduleParamList = new ArrayList<>(moduleParamsJson.size())

        for (moduleParamJson in moduleParamsJson) {
            ModuleParam moduleParam = createModuleParamFromJson(moduleParamJson)
            moduleParamList.add(moduleParam)
        }

        return moduleParamList
    }

    def createModuleParamFromJson(def moduleParamJson) {
        ModuleParam moduleParam = new ModuleParam()
        moduleParam.pKey = moduleParamJson.name
        moduleParam.pType = moduleParamJson.TYPE ?
                (moduleParamJson.TYPE.equalsIgnoreCase("file") ? "file" : "val") :
                "file"
        moduleParam.defaultVal = moduleParamJson.default_value || moduleParamJson.value
        moduleParam.descr = moduleParamJson.description
        moduleParam.pOrder = moduleParamJson.order
        moduleParam.pBasic = true

        return moduleParam
    }

}
