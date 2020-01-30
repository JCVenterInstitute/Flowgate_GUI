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
<g:form resource="${this.experiment}" method="PUT" class="col s12">
  <g:hiddenField name="version" value="${this.experiment?.version}"/>

  <div class="input-field col s12">
    <input type="text" name="title" required value="${experiment.title}"/>
    <label for="title">Title</label>
  </div>

  <div class="input-field col s12">
    <textarea name="description" class="materialize-textarea" required>${experiment.description}</textarea>
    <label for="description">Description</label>
  </div>

  <div class="input-field col s12">
    <textarea name="experimentHypothesis" class="materialize-textarea" required>${experiment.experimentHypothesis}</textarea>
    <label for="experimentHypothesis">Experiment Hypothesis</label>
  </div>

  <div class="input-field col s12">
    <textarea name="experimentMeta" class="materialize-textarea">${experiment.experimentMeta}</textarea>
    <label for="experimentMeta">Experiment Meta</label>
  </div>

  <div class="input-field col s12">
    <button type="submit" class="btn waves-effect waves-light">Update Experiment</button>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="btn-flat">Return to Experiment</a>
  </div>
</g:form>
</body>
</html>
