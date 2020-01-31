<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<content tag="topBtnBar">
  <g:hasErrors bean="${this.experiment}">
    <div class="row">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
        <g:eachError bean="${this.experiment}" var="error">
          <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
        </g:eachError>
      </div>
    </div>
  </g:hasErrors>
</content>

<h2>Create Experiment</h2>

<div class="row">
  <g:form action="save" class="col s12">
    <g:hiddenField name="datasets" value="${null}"/>
    <g:hiddenField name="experimentMeta" value=""/>
    <g:hiddenField name="pId" value="${pId}"/>
    <div class="row">
      <div class="input-field col s12">
        <input type="text" name="title" required/>
        <label for="title">Title</label>
      </div>

      <div class="input-field col s12">
        <textarea name="description" class="materialize-textarea" required></textarea>
        <label for="description">Description</label>
      </div>

      <div class="input-field col s12">
        <textarea name="experimentHypothesis" class="materialize-textarea" required></textarea>
        <label for="experimentHypothesis">Experiment Hypothesis</label>
      </div>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Create Experiment</button>
        <a href="${createLink(controller: 'project', action: 'index', params: [pId: pId])}" class="btn-flat">Return to Project</a>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
