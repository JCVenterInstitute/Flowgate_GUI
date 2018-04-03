<%@ page import="flowgate.User" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
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
