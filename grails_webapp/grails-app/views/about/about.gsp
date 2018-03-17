<!DOCTYPE html>
<html lang="en">
    <head>
        <meta name="layout" content="main">
        <title><g:message code="about.title.label" default="About" /></title>
    </head>
    <body class="bgimg">
        <div class="container">
            <g:render template="/shared/nav" />
            <div class="jumbotron" style="padding-top: 25px;">
                <div class="alert alert-warning" style="margin-top: 5px;">
                    The system is currently under testing and will be released in ...
                </div>
                <h4>
                    FlowGate is a web-based scientific portal designed for experimental scientists and data analysts who would like to run computational analysis of flow cytometry data but feel difficult to use an analytical workflow engine. It can be regarded as an advanced front-end for workflow-based analytical engines, which is hidden in the back-end but still accessible to the algorithm developers for adding methods and pipelines. FlowGate provides a graphical user interface for user registration, data upload, analysis invocation, visualization, and results download. Both the data processing and the time-consuming image generation procedure will be automatically parallelized in the back-end using the available compute resource in the cyber-infrastructure.<br/><br/>
                    FlowGate is designed for combined use with analytical workflow engines, which has been tested on multiple running environments, including virtual environments with multiple compute cores and shared memory and the Gordon parallel system at the San Diego Supercomputer Center. It can be deployed on XSEDE environment, a local cluster computer, a virtual machine in a Cloud, or even a desktop computer. It current works with two pipeline/workflow engines: the GenePattern analytical platform developed by the Broad Institute of MIT and Harvard and the bioKepler system developed at the University of California at San Diego. FlowGate currently supports FLOCK-like flow cytometry data analysis pipelines. We will soon expand it to support more workflow engines and other flow cytometry analytical pipelines and workflows.<br/><br/>
                    FlowGate has been developed by a group of immunological and computational scientists at the J. Craig Venter Institute and the San Diego Supercomputer Center.<br/><br/>
                    <sec:ifLoggedIn>
                    Click <button class="btn btn-primary btn-xs" onclick='window.location="${createLink(controller: 'project', action: 'list')}"'>here</button> to create or view your projects.
                    </sec:ifLoggedIn>
                </h4>
                </div>
        </div>
    </body>
</html>
