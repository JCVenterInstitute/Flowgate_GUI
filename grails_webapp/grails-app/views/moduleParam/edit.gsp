<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'moduleParam.label', default: 'ModuleParam')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
%{--<a href="#edit-moduleParam" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%

<div class="nav" role="navigation">
  %{--<ul>
    <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
    <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
    <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]"/></g:link></li>
  </ul>--}%
</div>

<div id="edit-moduleParam" class="container" role="main">
  <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="row justify-content-center ">
      <div class="alert alert-info text-center" role="alert">${flash.message}</div>
    </div>
  </g:if>
  <g:hasErrors bean="${this.moduleParam}">
    <ul class="errors" role="alert">
    <g:eachError bean="${this.moduleParam}" var="error">
      <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
    </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.moduleParam}" method="PUT" class="form-horizontal">
    <g:hiddenField name="version" value="${this.moduleParam?.version}"/>
    <div class="col-sm-4" style="padding-left: 0; margin-bottom: 5px;">
      <fieldset class="form">
        <f:all bean="moduleParam"/>
      </fieldset>

      <div class="col-sm-offset-4">
        <fieldset class="buttons">
          <input class="save btn btn-primary" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}"/>
          <a href="${createLink(controller: 'module', action: 'edit', params: [id: moduleParam.module.id])}" class="btn btn-warning">Back</a>
        </fieldset>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
