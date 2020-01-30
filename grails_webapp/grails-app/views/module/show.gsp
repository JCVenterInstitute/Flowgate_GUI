<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>

<div id="show-module" class="container" role="main">
  <h1 class="page-header"><g:message code="default.show.label" args="[entityName]"/></h1>
  <f:display bean="module"/>
  <g:form resource="${this.module}" method="DELETE">
    <g:link class="edit btn btn-primary" action="edit" resource="${this.module}"><g:message code="default.button.edit.label"
                                                                                            default="Edit"/></g:link>
    <input class="delete btn btn-warning" type="submit" value="${message(code: 'default.button.delete.label', default: 'Delete')}"
           onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');"/>
  </g:form>
</div>
</body>
</html>
