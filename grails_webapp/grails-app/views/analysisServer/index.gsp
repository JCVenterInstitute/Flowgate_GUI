<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>

<div id="list-analysisServer" class="content scaffold-list" role="main">
  <div class="row">
    <div class="col-sm-offset-1 col-sm-10">
      <h3 class="page-header"><g:message code="default.list.label" args="[entityName]"/></h3>

      <g:link class="btn btn-primary" style="margin-bottom: 10px;" action="create">New Server</g:link>
      <f:table collection="${analysisServerList}" properties="['name', 'url', 'userName']"/>
      <div class="pagination">
        <g:paginate total="${analysisServerCount ?: 0}"/>
      </div>
    </div>
  </div>
</div>
</body>
</html>