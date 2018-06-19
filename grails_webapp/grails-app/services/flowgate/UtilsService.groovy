package flowgate

import grails.converters.JSON
import grails.transaction.Transactional
import org.apache.commons.codec.binary.Base64
import org.grails.web.util.WebUtils

@Transactional
class UtilsService {

    def springSecurityService

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

    def periodicCheckNeeded(List<Integer> jobList){
        Boolean result = false
        if(jobList.size()>0){
            jobList.each {
                if(it.toInteger()>0 && Analysis.findByJobNumber(it.toInteger()).analysisStatus!=3){
                    result = true
                }
            }
        }
        return result
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

    def getProjectListForUser(User user, Map paginateParams) {
        if(user.username.equals("admin"))
            return Project.findAllByIsActive(true, [params: params])
        else {
            def projectUserList = ProjectUser.findAllByUser(user, [params: paginateParams]);
            List<Project> projectList = new ArrayList<Project>(projectUserList.size())
            for (def projectUser : projectUserList) {
                projectList.add(projectUser.project)
            }

            return projectList
        }
    }

}
