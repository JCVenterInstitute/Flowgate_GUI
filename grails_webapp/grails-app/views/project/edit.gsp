<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<content tag="topBtnBar">
  <div id="topBtnBar" class="row">
    <div class="pull-right">
      %{--
      <g:link class="btn btn-primary edit" controller="project" action="edit" id="${project?.id}">
          <i class="glyphicon glyphicon-edit"></i>Edit Project
      </g:link>
      <g:link class="btn btn-primary create" controller="project" action="create">
          <i class="glyphicon glyphicon-plus"></i>Add Project
      </g:link>
      <g:link class="btn btn-primary" controller="project" action="create" >
          <i class="glyphicon glyphicon-duplicate"></i>Clone Project
      </g:link>
      --}%
    </div>
  </div>
</content>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>

</content>
<content tag="pgContent">
  <div id="pageContent">
    <g:render template="templates/editTmpl"/>
  </div>
</content>
</body>
</html>
