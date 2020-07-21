<%@ page import="flowgate.ExperimentMetadataCategory" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment')}"/>
  <title><g:message code="default.mifcyt.annotation.label" default="MIFlowCyt Annotation"/></title>
</head>

<body>
<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="#!" class="breadcrumb dark"><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></a>
  </div>
</div>

<h2><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></h2>

<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped modal-trigger" href="#upload-annotation-file"
     data-tooltip="Upload an annotation file" data-position="left">
    <i class="material-icons">file_upload</i>
  </a>
</div>

<g:if test="${flash.error}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      M.toast({
        html: '<span>${flash.error}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
        displayLength: Infinity,
        classes: 'red'
      });
    });
  </script>
</g:if>

<g:render template="templates/mifcytMasterTbl" model="[category: null]"/>

</body>
</html>
