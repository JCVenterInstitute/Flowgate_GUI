<%@ page import="flowgate.User" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<g:render template="/shared/nav"/>
<content tag="topBtnBar">
  <g:if test="${flash.message}">
    <div class="row justify-content-center ">
      <div class="alert alert-info text-center" role="alert">${flash.message}</div>
    </div>
  </g:if>
  <g:hasErrors bean="${this.project}">
    <div class="row">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
        <g:eachError bean="${this.project}" var="error">
          <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
        </g:eachError>
      </div>
    </div>
  </g:hasErrors>
</content>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>
</content>
<content tag="pgContent">
  <div id="pageContent">
    <g:render template="templates/createTmpl"/>
  </div>
</content>
</body>
</html>
