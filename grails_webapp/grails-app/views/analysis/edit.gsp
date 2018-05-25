<%@ page import="flowgate.Analysis; flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}" />
  <title><g:message code="default.edit.label" args="[entityName]" /></title>
</head>

<body>
%{--<a href="#edit-analysis" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%
<div class="nav" role="navigation">
  %{--<ul>--}%
    %{--<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><a class="home" href="${createLink(controller: 'experiment', action: 'index', params: [eId: this.analysis.experiment.id])}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><a class="home" href="${createLink(uri: '/experiment/index?eId='+ this.analysis.experiment.id)}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><g:link class="list" action="index" params="[eId: this.analysis.experiment.id]" ><g:message code="default.list.label" args="[entityName]" /></g:link></li>--}%
    %{--<li><g:link class="create" action="create" params="[eId: this.analysis.experiment.id]"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
  %{--</ul>--}%
</div>
<div id="edit-analysis" class="content scaffold-edit" role="main">
  <ul class="breadcrumb">
    <li><a href="/flowgate/project/index?pId=${analysis?.experiment?.project?.id}" title="${analysis?.experiment?.project?.title}">${analysis?.experiment?.project?.title}</a></li>
    <li><a href="/flowgate/experiment/index?eId=${analysis?.experiment?.id}" title="${analysis?.experiment?.title}">${analysis?.experiment?.title}</a></li>
    <li class="active">Analysis</li>
  </ul>
  <h3 class="text-center"><g:message code="default.edit.label" args="[entityName]" /></h3>
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
  <g:form resource="${this.analysis}" method="PUT">
    <g:hiddenField name="version" value="${this.analysis?.version}" />
    <fieldset class="form">
    <f:with bean="analysis">
      <div class="fieldcontain">
        <label for="exp">Experiment</label>
        <f:display id="exp" property="experiment.title" />
        <g:hiddenField name="experimentId" value="${this.analysis.experiment}" />
      </div>
      <f:field property="analysisName" required="true"/>
      <f:field property="analysisDescription" required="false"/>
      <f:field property="analysisStatus" required="false"/>
      %{--<f:field property="jobNumber" disabled="true" />--}%
      <div class="fieldcontain">
        <label for="jobNumber">Job Number</label>
        <f:display id="jobNumber" property="jobNumber" />
        <g:hiddenField name="jobNumber" value="${this.analysis.jobNumber}" />
      </div>

      <div class="fieldcontain">
        <label for="usr">Sent by</label>
        <f:display id="usr" property="user.username" />
        <g:hiddenField name="userId" value="${this.analysis.user}" />
      </div>

      <div class="fieldcontain">
        <label for="tstmp">Date Created</label>
        <f:display id="tstmp" property="timestamp" />
        <g:hiddenField name="timestamp" value="${this.analysis.timestamp}" />
      </div>
    </f:with>
  </fieldset>
  %{--<fieldset class="buttons">--}%
  <fieldset class="text-center">
    <input class="btn btn-success save" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}" />
  </fieldset>
</g:form>
</div>
</body>
</html>
