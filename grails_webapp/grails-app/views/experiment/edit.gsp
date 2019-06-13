<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>
</content>
<content tag="pgContent">
  <div id="pageContent">
    <g:render template="templates/editTmpl" model="[experiment: experiment]"/>
  </div>
</content>
</body>
</html>
