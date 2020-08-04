<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>

<g:hasErrors bean="${this.project}">
  <div class="row">
    <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
      <g:eachError bean="${this.project}" var="error">
        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </div>
  </div>
</g:hasErrors>


<h2><g:message code="default.create.label" args="[entityName]"/></h2>

<div class="row">
  <g:form action="save" class="col s12">
    <f:with bean="project">
      <div class="row">
        <f:field property="title" required="true"/>
        <f:field property="description" type="textarea" required="true"/>
      </div>
    </f:with>
    <div class="row">
      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Create Project</button>
        <g:if test="${!flash.firstTime}">
          <a href="${createLink(controller: 'project', action: 'list')}" class="btn-flat">Return to Project List</a>
        </g:if>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
