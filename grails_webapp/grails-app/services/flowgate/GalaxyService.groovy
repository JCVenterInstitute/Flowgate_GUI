package flowgate

import com.github.jmchilton.blend4j.galaxy.GalaxyInstance
import com.github.jmchilton.blend4j.galaxy.GalaxyInstanceFactory
import com.github.jmchilton.blend4j.galaxy.HistoriesClient
import com.github.jmchilton.blend4j.galaxy.JobsClient
import com.github.jmchilton.blend4j.galaxy.LibrariesClient
import com.github.jmchilton.blend4j.galaxy.WorkflowsClient
import com.github.jmchilton.blend4j.galaxy.beans.FileLibraryUpload
import com.github.jmchilton.blend4j.galaxy.beans.History
import com.github.jmchilton.blend4j.galaxy.beans.JobDetails
import com.github.jmchilton.blend4j.galaxy.beans.Library
import com.github.jmchilton.blend4j.galaxy.beans.LibraryContent
import com.github.jmchilton.blend4j.galaxy.beans.LibraryFolder
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowDetails
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInputDefinition
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInputs
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInvocation
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowInvocationInputs
import com.github.jmchilton.blend4j.galaxy.beans.WorkflowOutputs
import com.sun.jersey.api.client.ClientResponse
import grails.util.Holders
import grails.web.servlet.mvc.GrailsParameterMap
import org.codehaus.jackson.map.ObjectMapper
import org.codehaus.jackson.type.TypeReference

class GalaxyService {
    final GalaxyInstance galaxyInstance
    private LibrariesClient librariesClient
    private WorkflowsClient workflowsClient
    private HistoriesClient historiesClient
    private JobsClient jobsClient
    final def flowgateLibrary = "FlowGate"
    final def flowgateHistoryName = "FlowGate"
    protected final ObjectMapper mapper

    ClientResponse clientResponse;
    def grailsApplication = Holders.grailsApplication

    public GalaxyService(AnalysisServer server) {
        galaxyInstance = GalaxyInstanceFactory.getFromCredentials(server.url, server.userName, server.userPw)
        mapper = new ObjectMapper();
    }

    private LibrariesClient getLibrariesClient() {
        if(librariesClient == null) {
            librariesClient = galaxyInstance.getLibrariesClient();
        }

        return librariesClient;
    }

    private WorkflowsClient getWorkflowsClient() {
        if(workflowsClient == null) {
            workflowsClient = galaxyInstance.getWorkflowsClient();
        }

        return workflowsClient;
    }

    private JobsClient getJobsClient() {
        if(jobsClient == null) {
            jobsClient = galaxyInstance.getJobsClient();
        }

        return jobsClient;
    }

    private HistoriesClient getHistoriesClient() {
        if(historiesClient == null) {
            historiesClient = galaxyInstance.getHistoriesClient();
        }

        return historiesClient;
    }

    def createLibraryFolder(def libraryId, def folderName, def folderDescription, boolean isRootLibrary) {
        final LibraryFolder folder = new LibraryFolder();
        folder.setDescription(folderDescription);
        folder.setName(folderName);

        if (isRootLibrary) {
            final LibraryContent rootFolder = getLibrariesClient().getRootFolder(libraryId);
            folder.setFolderId(rootFolder.getId());
        } else {
            folder.setFolderId(libraryId)
        }

        clientResponse = getLibrariesClient().createFolderRequest(libraryId, folder)
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
            fileName = fileName.substring(0, lastDot) + "-" + UUID.randomUUID().toString() + fileName.substring(lastDot)
        }
        upload.setName(fileName)

        clientResponse = getLibrariesClient().uploadFile(libraryOrFolderId, upload);
        if (clientResponse.getStatus() == 200)
            return mapper.readValue(clientResponse.getEntity(String.class), new TypeReference<List<FileLibraryUpload>>() {}).get(0)
        else
            throw new Exception(clientResponse)
    }

    def getFlowGateHistoryId() {
        final List<History> histories = getHistoriesClient().getHistories();
        Optional<History> optHistory = histories.stream()
                .filter({ history -> history.getName().equals(flowgateHistoryName) })
                .findFirst();

        if(optHistory.isPresent())
            return optHistory.get().getId();
        else
            return null;
    }

    def downloadFile(String fileId) {
        String historyId = getFlowGateHistoryId();
        return getHistoriesClient().returnDataset(historyId, fileId)
    }

    def fetchImmportGalaxyWorkflows() {
        return getWorkflowsClient().getWorkflows();
    }

    def fetchImmportGalaxyWorkflowInputs(def moduleName) {
        clientResponse = getWorkflowsClient().showWorkflowResponse(moduleName)

        if (clientResponse.getStatus() == 200) {
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

    def submitImmportGalaxyWorkflow(Module module, Experiment experiment, GrailsParameterMap params, def request) throws UnknownHostException {
        //Get FlowGate Library
        List<Library> libraries = getLibrariesClient().getLibraries();
        Optional<Library> libraryOpt = libraries.stream()
                .filter({ library -> library.getName().equals(flowgateLibrary) })
                .findFirst()

        if (libraryOpt.isPresent()) {
            Library library = libraryOpt.get()
            List<LibraryContent> libraryContents = getLibrariesClient().getLibraryContents(library.id)

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
            boolean hasMultipleFCSFiles = false;
            def fileIds;
            def fileOrder;

            String flowgateHistoryId = getFlowGateHistoryId();
            if (null != flowgateHistoryId) {
                inputs.setDestination(new WorkflowInputs.ExistingHistory(flowgateHistoryId)) //submit the workflow on FlowGate history
                inputs.setWorkflowId(module.name)

                for (final ModuleParam moduleParam : module.moduleParams) {
                    if (moduleParam.pType.equals("ds")) {
                        //get Dataset that user selected
                        def dsId = params["mp-${moduleParam.id}-ds"]
                        Dataset ds = Dataset.get(dsId.toLong())

                        if (ds) {
                            def fileSize = ds.expFiles.size();
                            fileIds = new String[fileSize]
                            hasMultipleFCSFiles = fileSize > 1;

                            ds.expFiles.eachWithIndex { expFile, index ->
                                Optional<LibraryContent> libraryContentOpt = libraryContents.stream()
                                        .filter({ libraryContent -> libraryContent.getName().equals("/" + experiment.project.id + "/" + experiment.id + "/" + expFile.fileName) })
                                        .findFirst()
                                if (libraryContentOpt.isPresent()) {
                                    fileIds.putAt(index, libraryContentOpt.get().id)
                                } else {
                                    def FileLibraryUpload uploadFile = uploadFileToLibraryOrFolder(experimentFolderId, expFile.filePath, expFile.fileName)
                                    fileIds.putAt(index, uploadFile.id)
                                }
                            }
                            inputs.setInput(moduleParam.pOrder.toString(), new WorkflowInputs.WorkflowInput(fileIds[0], WorkflowInputs.InputSourceType.LD))
                            fileOrder = moduleParam.pOrder.toString();
                        }
                    } else {
                        def partFile = request.getFile("mp-${moduleParam.id}")
                        def FileLibraryUpload uploadFile = uploadFileToLibraryOrFolder(experimentFolderId, partFile.part.fileItem.tempFile, partFile.filename)

                        inputs.setInput(moduleParam.pOrder.toString(), new WorkflowInputs.WorkflowInput(uploadFile.id, WorkflowInputs.InputSourceType.LD))
                    }

                    def config = grailsApplication.config;
                    def isCustomUCIPipeline = module.title.equals(config.getProperty('UCI.workflow', String))

                    if (isCustomUCIPipeline) {
                        def libFolder = config.getProperty('UCI.libraryFolder', String)
                        def incFile = config.getProperty('UCI.inclusion.name', String)
                        def excFile = config.getProperty('UCI.exclusion.name', String)
                        def headerLstFile = config.getProperty('UCI.headerList.name', String)
                        def headerRplcFile = config.getProperty('UCI.headerReplace.name', String)

                        for (LibraryContent libraryContent : libraryContents) {
                            if (libraryContent.getName().equals("/" + libFolder + "/" + incFile)) {
                                inputs.setInput(config.getProperty('UCI.inclusion.order', String),
                                        new WorkflowInputs.WorkflowInput(libraryContent.id, WorkflowInputs.InputSourceType.LD))
                            } else if (libraryContent.getName().equals("/" + libFolder + "/" + excFile)) {
                                inputs.setInput(config.getProperty('UCI.exclusion.order', String),
                                        new WorkflowInputs.WorkflowInput(libraryContent.id, WorkflowInputs.InputSourceType.LD))
                            } else if (libraryContent.getName().equals("/" + libFolder + "/" + headerLstFile)) {
                                inputs.setInput(config.getProperty('UCI.headerList.order', String),
                                        new WorkflowInputs.WorkflowInput(libraryContent.id, WorkflowInputs.InputSourceType.LD))
                            } else if (libraryContent.getName().equals("/" + libFolder + "/" + headerRplcFile)) {
                                inputs.setInput(config.getProperty('UCI.headerReplace.order', String),
                                        new WorkflowInputs.WorkflowInput(libraryContent.id, WorkflowInputs.InputSourceType.LD))
                            }
                        }
                    }
                }
            } else {
                throw new Exception("Please create a history named 'FlowGate' in ImmportGalaxy");
            }

            if (!hasMultipleFCSFiles) {
                //Run Workflow
                clientResponse = getWorkflowsClient().runWorkflowResponse(inputs);
                if (clientResponse.getStatus() == 200) {
                    WorkflowOutputs output = clientResponse.getEntity(WorkflowOutputs.class);
                    System.out.println("Running workflow in history " + output.getHistoryId());
                    System.out.println("Workflow output id: " + output.id);

                    return output.id
                } else {
                    throw new Exception("Workflow couldn't be created!")
                }
            } else {
                StringJoiner outputIds = new StringJoiner(",");
                List<WorkflowInputs> workflowInputs = Collections.nCopies(fileIds.length, inputs);
                for(int i=0; i<workflowInputs.size(); ++i) {
                    WorkflowInputs workflowInput = workflowInputs.get(i);
                    //First one is already defined
                    if(i != 0) {
                        workflowInput
                                .setInput(fileOrder, new WorkflowInputs.WorkflowInput(fileIds[i], WorkflowInputs.InputSourceType.LD))
                    }

                    //Run Workflow
                    clientResponse = getWorkflowsClient().runWorkflowResponse(workflowInput);
                    if (clientResponse.getStatus() == 200) {
                        WorkflowOutputs output = clientResponse.getEntity(WorkflowOutputs.class);
                        System.out.println("Running workflow in history " + output.getHistoryId());
                        System.out.println("Workflow output id: " + output.id);

                        outputIds.add(output.id);
                    } else {
                        throw new Exception("Workflow couldn't be created!")
                    }
                }

                return outputIds.toString();
            }
        } else {
            throw new Exception(flowgateLibrary + " library doesn't exist in ImmportGalaxy")
        }
    }

    def getInvocationStatus(String workflowId, String invocationIds) {
        String[] invocationIdArr = invocationIds.split(",");
        JobDetails[] jobDetailsArr = new JobDetails[invocationIdArr.length];

        for (int i = 0; i < invocationIdArr.length; ++i) {
            WorkflowInvocation workflowInvocation = getWorkflowsClient().showInvocation(workflowId, invocationIdArr[i])
            List<WorkflowInvocation.WorkflowInvocationStep> steps = workflowInvocation.getSteps()
            Collections.sort(steps)

            for (int j = 0; j < steps.size(); ++j) {
                WorkflowInvocation.WorkflowInvocationStep step = steps.get(j)

                if (step.jobId != null) {
                    JobDetails jobDetails = getJobsClient().showJob(step.jobId)

                    if (jobDetails.getState().equals("error") || j == steps.size() - 1) {
                        jobDetailsArr.putAt(i, jobDetails)
                    }
                }
            }
        }

        return jobDetailsArr
    }

    def getFileNameFromInvocation(String workflowId, String invocationId) {
        String historyId = getFlowGateHistoryId()
        WorkflowInvocation workflowInvocation = getWorkflowsClient().showInvocation(workflowId, invocationId)
        Map<String, WorkflowInvocationInputs.WorkflowInvocationInput> workflowInvocationInputMap = workflowInvocation.getInputs();
        WorkflowInvocationInputs.WorkflowInvocationInput input = workflowInvocationInputMap.get("0");
        com.github.jmchilton.blend4j.galaxy.beans.Dataset dataset = getHistoriesClient().showDataset(historyId, input.id)

        return dataset.name;
    }

}
