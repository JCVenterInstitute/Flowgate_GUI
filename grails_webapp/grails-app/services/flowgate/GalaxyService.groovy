package flowgate

import com.github.jmchilton.blend4j.galaxy.GalaxyInstance
import com.github.jmchilton.blend4j.galaxy.GalaxyInstanceFactory
import com.github.jmchilton.blend4j.galaxy.HistoriesClient
import com.github.jmchilton.blend4j.galaxy.JobsClient
import com.github.jmchilton.blend4j.galaxy.LibrariesClient
import com.github.jmchilton.blend4j.galaxy.WorkflowsClient
import com.github.jmchilton.blend4j.galaxy.beans.FileLibraryUpload
import com.github.jmchilton.blend4j.galaxy.beans.JobDetails
import com.github.jmchilton.blend4j.galaxy.beans.Library
import com.github.jmchilton.blend4j.galaxy.beans.LibraryContent
import com.github.jmchilton.blend4j.galaxy.beans.LibraryFolder
import com.github.jmchilton.blend4j.galaxy.beans.Workflow
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowDetails
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInputDefinition
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInputs
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInvocation
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowOutputs
import com.sun.jersey.api.client.ClientResponse
import grails.web.servlet.mvc.GrailsParameterMap
import org.codehaus.jackson.map.ObjectMapper
import org.codehaus.jackson.type.TypeReference

class GalaxyService {
    final GalaxyInstance galaxyInstance
    final LibrariesClient librariesClient
    final WorkflowsClient workflowsClient
    final HistoriesClient historiesClient
    final JobsClient jobsClient
    final def flowgateLibrary = "FlowGate"
    final def flowgateHistoryId = "ba751ee0539fff04"
    protected final ObjectMapper mapper

    ClientResponse clientResponse;

    public GalaxyService(AnalysisServer server) {
        galaxyInstance = GalaxyInstanceFactory.getFromCredentials(server.url, server.userName, server.userPw)
        librariesClient = galaxyInstance.getLibrariesClient()
        workflowsClient = galaxyInstance.getWorkflowsClient()
        jobsClient = galaxyInstance.getJobsClient()
        historiesClient = galaxyInstance.getHistoriesClient()
        mapper = new ObjectMapper();
    }

    def createLibraryFolder(def libraryId, def folderName, def folderDescription, boolean isRootLibrary) {
        final LibraryFolder folder = new LibraryFolder();
        folder.setDescription(folderDescription);
        folder.setName(folderName);

        if (isRootLibrary) {
            final LibraryContent rootFolder = librariesClient.getRootFolder(libraryId);
            folder.setFolderId(rootFolder.getId());
        } else {
            folder.setFolderId(libraryId)
        }

        clientResponse = librariesClient.createFolderRequest(libraryId, folder)
        if (clientResponse.getStatus() == 200)
            return mapper.readValue(clientResponse.getEntity(String.class), new TypeReference<List<LibraryFolder>>() {}).get(0);
        else
            throw new Exception(clientResponse)
    }

    def uploadFileToLibraryOrFolder(def libraryOrFolderId, String filePath, def fileName) {
        File fileToUpload = new File(filePath + fileName)
        uploadFileToLibraryOrFolder(libraryOrFolderId, fileToUpload, fileName)
    }

    def uploadFileToLibraryOrFolder(def libraryOrFolderId, File fileToUpload, def fileName) {
        // upload file to Library
        FileLibraryUpload upload = new FileLibraryUpload()
        upload.setFolderId(libraryOrFolderId)

        upload.setFile(fileToUpload)
        if (fileName.endsWith(".fcs")) {
            upload.setFileType("fcs")
        } else {
            int lastDot = fileName.lastIndexOf('.')
            fileName = fileName.substring(0,lastDot) + "-" + UUID.randomUUID().toString() + fileName.substring(lastDot)
        }
        upload.setName(fileName)

        clientResponse = librariesClient.uploadFile(libraryOrFolderId, upload);
        if (clientResponse.getStatus() == 200)
            return mapper.readValue(clientResponse.getEntity(String.class), new TypeReference<List<FileLibraryUpload>>() {}).get(0)
        else
            throw new Exception(clientResponse)
    }

    def downloadFile(String fileId) {
        return historiesClient.returnDataset(flowgateHistoryId, fileId)
    }

    def fetchImmportGalaxyWorkflows() {
       return workflowsClient.getWorkflows();
    }

    def fetchImmportGalaxyWorkflowInputs(def moduleName) {
        clientResponse = workflowsClient.showWorkflowResponse(moduleName)

        if (clientResponse.getStatus() == 200){
            WorkflowDetails workflowDetails = clientResponse.getEntity(WorkflowDetails.class)
            Map<String, WorkflowInputDefinition> workflowInputs = workflowDetails.inputs
            def attributes = workflowInputs.collect { key, input ->
                ["value": input.value, "name": input.label, "order": Integer.parseInt(key)]
            }
            return attributes
        } else {
            throw new Exception(clientResponse)
        }
    }

    def submitImmportGalaxyWorkflow(Module module, Experiment experiment, GrailsParameterMap params, def request) {
        //Get FlowGate Library
        List<Library> libraries = librariesClient.getLibraries();
        Optional<Library> libraryOpt = libraries.stream()
                .filter({ library -> library.getName().equals(flowgateLibrary) })
                .findFirst()

        if (libraryOpt.isPresent()) {
            Library library = libraryOpt.get()
            List<LibraryContent> libraryContents = librariesClient.getLibraryContents(library.id)

            def projectFolderId
            def experimentFolderId
            for (final LibraryContent libraryContent : libraryContents) {
                if (libraryContent.name.equals("/" + experiment.project.id)) {
                    projectFolderId = libraryContent.id;
                } else if (libraryContent.name.equals("/" + experiment.project.id + "/" + experiment.id)) {
                    experimentFolderId = libraryContent.id;
                }
            }

            //create project folder if it hasn't been created
            if (!projectFolderId) {
                final LibraryFolder projectFolder = createLibraryFolder(library.id, experiment.project.id.toString(), experiment.project.title, true)
                projectFolderId = projectFolder.id
            }

            //create experiment folder if it hasn't been created
            if (!experimentFolderId) {
                final LibraryFolder experimentFolder = createLibraryFolder(projectFolderId, experiment.id.toString(), experiment.title, false)
                experimentFolderId = experimentFolder.id
            }

            final WorkflowInputs inputs = new WorkflowInputs()
            inputs.setDestination(new WorkflowInputs.ExistingHistory(flowgateHistoryId)) //submit the workflow on FlowGate history
            inputs.setWorkflowId(module.name)

            for (final ModuleParam moduleParam : module.moduleParams) {
                if (moduleParam.pType.equals("ds")) {
                    //get Dataset that user selected
                    def dsId = params["mp-${moduleParam.id}-ds"]
                    Dataset ds = Dataset.get(dsId.toLong())

                    if (ds) {
                        //def fileIds = new String[ds.expFiles.size()]
                        def fileId

                        ds.expFiles.eachWithIndex { expFile, index ->
                            Optional<LibraryContent> libraryContentOpt = libraryContents.stream()
                                    .filter({ libraryContent -> libraryContent.getName().equals("/" + experiment.project.id + "/" + experiment.id + "/" + expFile.fileName) })
                                    .findFirst()
                            if (libraryContentOpt.isPresent()) {
                                //fileIds.putAt(index, libraryContentOpt.get().id)
                                fileId = libraryContentOpt.get().id
                            } else {
                                def FileLibraryUpload uploadFile = uploadFileToLibraryOrFolder(experimentFolderId, expFile.filePath, expFile.fileName)
                                //fileIds.putAt(index, uploadFile.id)
                                fileId = uploadFile.id
                            }
                        }
                        inputs.setInput(moduleParam.pOrder.toString(), new WorkflowInputs.WorkflowInput(fileId, WorkflowInputs.InputSourceType.LD))
                    }
                } else {
                    def partFile = request.getFile("mp-${moduleParam.id}")
                    def FileLibraryUpload uploadFile = uploadFileToLibraryOrFolder(experimentFolderId, partFile.part.fileItem.tempFile, partFile.filename)

                    inputs.setInput(moduleParam.pOrder.toString(), new WorkflowInputs.WorkflowInput(uploadFile.id, WorkflowInputs.InputSourceType.LD))
                }
            }

            //Run Workflow
            clientResponse = workflowsClient.runWorkflowResponse(inputs);
            if (clientResponse.getStatus() == 200) {
                WorkflowOutputs output = clientResponse.getEntity(WorkflowOutputs.class);
                System.out.println("Running workflow in history " + output.getHistoryId());
                for (String outputId : output.getOutputIds()) {
                    System.out.println("Workflow writing to output id " + outputId);
                }

                return output.id
            } else {
                throw new Exception("Workflow couldn't be created!")
            }
        } else {
            throw new Exception(flowgateLibrary + " library doesn't exist in ImmportGalaxy")
        }
    }

    def getInvocationStatus(String workflowId, String invocationId) {
        WorkflowInvocation workflowInvocation = workflowsClient.showInvocation(workflowId, invocationId)
        List<WorkflowInvocation.WorkflowInvocationStep> steps = workflowInvocation.getSteps()
        Collections.sort(steps)

        for (int i = 0; i < steps.size(); i++) {
            WorkflowInvocation.WorkflowInvocationStep step = steps.get(i)

            if (step.jobId != null) {
                JobDetails jobDetails = jobsClient.showJob(step.jobId)

                if (jobDetails.getState().equals("error") || i == steps.size()-1){
                    return jobDetails
                }
            }
        }
    }
}
