<%@ page import="flowgate.Analysis; flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>

<div class="container">
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: analysis?.experiment?.project?.id])}" title="${analysis?.experiment?.project?.title}">${analysis?.experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: analysis?.experiment?.id])}" title="${analysis?.experiment?.title}">${analysis?.experiment?.title}</a></li>
    <li class="active">Edit Analysis</li>
  </ul>

  <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
  <g:hasErrors bean="${this.analysis}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.analysis}" var="error">
        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.analysis}" method="PUT" class="form-horizontal">
    <g:hiddenField name="version" value="${this.analysis?.version}"/>

    <f:with bean="analysis">
      <div class="form-group">
        <label class="col-sm-2 control-label" for="exp">Experiment</label>

        <div class="col-sm-10"><f:display id="exp" property="experiment.title"/></div>
        <g:hiddenField name="experimentId" value="${this.analysis.experiment.id}"/>
      </div>
      <f:field property="analysisName" required="true"/>
      <f:field property="analysisDescription" required="false"/>
    %{--<f:field property="jobNumber" disabled="true" />--}%
      <div class="form-group">
        <label class="col-sm-2 control-label" for="jobNumber">Job Number</label>

        <div class="col-sm-10"><f:display id="jobNumber" property="jobNumber"/></div>
        <g:hiddenField name="jobNumber" value="${this.analysis.jobNumber}"/>
      </div>

      <div class="form-group">
        <label class="col-sm-2 control-label" for="usr">Sent by</label>

        <div class="col-sm-10"><f:display id="usr" property="user.username"/></div>
        <g:hiddenField name="userId" value="${this.analysis.user}"/>
      </div>

      <div class="form-group">
        <label class="col-sm-2 control-label" for="tstmp">Date Created</label>

        <div class="col-sm-10"><f:display id="tstmp" property="timestamp"/></div>
        <g:hiddenField name="timestamp" value="${this.analysis.timestamp}"/>
      </div>
    </f:with>

    <div class="form-group">
      <div class="col-sm-offset-2 col-sm-10">
        <g:submitButton name="update" class="btn btn-success" value="${message(code: 'default.button.update.label', default: 'Update')}"/>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
