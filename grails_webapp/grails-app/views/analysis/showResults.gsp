<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}" />
  <title><g:message code="default.show.label" args="[entityName]" /></title>
  <style>
    footer{
      z-index: 101;
    }
    .fieldcontain{
      padding-left: 30px;
    }
  </style>
</head>

<body>
<div class="nav" role="navigation"></div>
<g:if test="${flash?.resultMsg}">
  <br/>
  <br/>
  <br/>
  <div class="text-center">
    <div class="alert alert-success alert-dismissible" role="alert">
      <button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>
      <strong>Error in Job Result!</strong>&nbsp;&nbsp;${flash?.resultMsg}
    </div>
  </div>
</g:if>
<g:else>
  <div class="container">
    <ul class="breadcrumb">
      <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: analysis?.experiment?.project?.id])}" title="${analysis?.experiment?.project?.title}">${analysis?.experiment?.project?.title}</a></li>
      <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: analysis?.experiment?.id])}" title="${analysis?.experiment?.title}">${analysis?.experiment?.title}</a></li>
      <li><a href="${createLink(controller: 'analysis', action: 'index', params: [eId: analysis?.experiment?.id])}">List of Analysis Task</a></li>
      <li class="active">Analysis Results</li>
    </ul>
    <h1 class="page-header"><g:message code="analysis.showResult.label" args="[entityName]" default="Analysis Results" /></h1>
    <g:if test="${flash.message}">
      <div class="message" role="status">${flash.message}</div>
    </g:if>
    <f:with bean="analysis">
      <dl class="dl-horizontal">
        <dt>Experiment:</dt>
        <dd>${analysis?.experiment?.title}</dd>

        <dt>Analysis Name:</dt>
        <dd>${analysis?.analysisName}</dd>

        <dt>Analysis Description:</dt>
        <dd>${analysis?.analysisDescription}</dd>

        <dt>Analysis Status:</dt>
        <dd>${analysis.analysisStatus == 1 ? 'init' : analysis.analysisStatus == 2 ? 'pending': analysis.analysisStatus == 3 ? 'results ready' : analysis.analysisStatus == -1 ? 'error' : 'done'}</dd>

        <dt>Job Number:</dt>
        <dd>${analysis.jobNumber.toString()}</dd>

        <dt>Sent by:</dt>
        <dd>${analysis?.user.username}</dd>

        <dt>Date Created:</dt>
        <dd><g:formatDate id="tstmp" date="${analysis.timestamp}" format="MM/dd/yyyy hh:mm:ss"/></dd>

        <dt>Date Completed:</dt>
        <g:if test="${jobResult.dateCompleted}">
          <dd><g:formatDate id="jobComplete" date="${new Date().parse("yyyy-MM-dd'T'hh:mm:ss",jobResult?.dateCompleted)}" format="MM/dd/yyyy hh:mm:ss"/></dd>
        </g:if>

        <dt>Job Complete Status:</dt>
        <dd><i class=" fa fa-circle" style="color: ${!jobResult?.status?.hasError ? 'lawngreen' : 'red'}"></i>${jobResult?.status?.statusMessage}</dd>

        <g:render template="results/resultsReport" />
      </dl>
    </f:with>
    <div class="row" style="max-width: 100%">
      <div class="col-sm-12">
        <sec:ifAnyGranted roles="ROLE_Administrator">
          <g:render template="results/resultsFileLst" />
        %{--<g:render template="results/resultsGrid" />--}%
        </sec:ifAnyGranted>
      </div>
    </div>
    %{--TODO remove for normal user / change visibility    --}%
    <g:form resource="${this.analysis}" method="DELETE">
      <div class="form-group">
        <g:link class="edit btn btn-primary" action="edit" resource="${this.analysis}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
        <input class="delete btn btn-danger" type="submit" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
      </div>
    </g:form>
  </div>
</g:else>
</body>
</html>
