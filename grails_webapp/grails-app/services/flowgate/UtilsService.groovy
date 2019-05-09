package flowgate

import grails.converters.JSON
import grails.plugin.springsecurity.SpringSecurityUtils
import grails.transaction.Transactional
import grails.web.servlet.mvc.GrailsParameterMap
import org.apache.commons.codec.binary.Base64
import org.grails.web.util.WebUtils

@Transactional
class UtilsService {

    def springSecurityService
    def restUtilsService
    def wsService

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
        return new String(Base64.encodeBase64((username+':'+password).getBytes()), "UTF-8")
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
      return Analysis.findAllByExperimentAndAnalysisStatusNotInList(experiment, [3,-1])*.jobNumber
    }
    else {
      return Analysis.findAllByExperimentAndUserAndAnalysisStatusNotInList(experiment, springSecurityService.currentUser, [3,-2,-1])*.jobNumber
    }
  }

  def checkJobStatus(def jobLst){
    jobLst.each { jobId ->
      Analysis analysis = Analysis.findByJobNumber(jobId.toInteger())
      if(jobId.toInteger() > 0){
        Boolean completed = restUtilsService.isComplete(analysis)
        if(completed){
          analysis.analysisStatus = jobId.toInteger() > 0 ? completed ? 3 : 2 : jobId.toInteger()
          analysis.save flush: true
          wsService.tcMsg(jobId.toString())
        }
      }
    }
  }


}
