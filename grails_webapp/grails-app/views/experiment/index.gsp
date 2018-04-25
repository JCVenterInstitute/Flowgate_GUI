<%@ page import="flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.experiment.label" args="[entityName]" default="${entityName}"/></title>
  <asset:javascript src="jquery-2.2.0.min.js"/>
</head>

<body>
<g:render template="/shared/nav"/>
<content tag="topBtnBar">
  <div id="topBtnBar">
    <div class="row">
      <div class="right">
        <g:render template="templates/indexTopBtnBar" model="[experiment: experiment]"/>
      </div>
    </div>
  </div>
  <g:render template="/shared/errorsMsgs" model="[bean: this.experiment]"/>
</content>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>
</content>
<content tag="pgContent">
  <div id="pageContent">
    <g:render template="templates/indexTmpl" model="[experiment: experiment]"/>
  </div>
</content>
</body>
</html>