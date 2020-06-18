<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.projectView.label" default="Project" args="[entityName]"/></title>
</head>

<body>
<div class="row">
  <content tag="topBtnBar">
    <div id="topBtnBar">
      <g:render template="templates/indexTopBtnBar" model="[project: this.project]"/>
    </div>
    <g:render template="/shared/errorsMsgs" model="[bean: this.project]"/>
  </content>

  <div id="pageContent">
    <g:render template="templates/indexTmpl" model="[experimentList: experimentList]"/>
  </div>
</div>
</body>
</html>