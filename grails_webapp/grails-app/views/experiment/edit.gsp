<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.edit.label" args="[entityName]"/></h2>
<g:hasErrors bean="${this.experiment}">
  <div class="row m-0">
    <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
      <g:eachError bean="${this.experiment}" var="error">
        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </div>
  </div>
</g:hasErrors>
<div class="row">
  <g:form resource="${this.experiment}" method="PUT" class="col s12">
    <g:hiddenField name="version" value="${this.experiment?.version}"/>

    <f:with bean="experiment">
      <div class="row">
        <f:field property="title" required="true" value="${experiment.title}"/>
        <f:field property="description" type="textarea" required="true" value="${experiment.description}"/>
        <f:field property="experimentHypothesis" type="textarea" value="${experiment.experimentHypothesis}"/>
        <f:field property="experimentMeta" type="textarea" value="${experiment.experimentMeta}"/>
      </div>
    </f:with>

    <div class="row">
      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Update Experiment</button>
        <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="btn-flat">Return to Experiment</a>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
