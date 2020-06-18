<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.edit.label" args="[entityName]"/></h2>

<g:hasErrors bean="${this.project}">
  <ul class="errors" role="alert">
  <g:eachError bean="${this.project}" var="error">
    <li<g:if
      test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message
      error="${error}"/></li>
  </g:eachError>
  </ul>
</g:hasErrors>
<div class="row">
  <g:form action="update" resource="${this.project}" method="POST" class="col s12">
    <g:hiddenField name="version" value="${this.project?.version}"/>
    <div class="row">
      <div class="input-field col s12">
        <input type="text" name="title" required value="${project.title}"/>
        <label for="title">Title</label>
      </div>

      <div class="input-field col s12">
        <textarea name="description" class="materialize-textarea" required>${project.description}</textarea>
        <label for="description">Description</label>
      </div>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Update Project</button>
        <a href="${createLink(controller: 'project', action: 'index', params: [pId: project?.id])}" class="btn-flat">Return to Project</a>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
