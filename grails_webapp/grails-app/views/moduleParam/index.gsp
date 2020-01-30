<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'moduleParam.label', default: 'ModuleParam')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>

<div id="list-moduleParam" class="content scaffold-list" role="main">
  <h1><g:message code="default.list.label" args="[entityName]"/></h1>
  <f:table collection="${moduleParamList}"/>

  <div class="pagination">
    <g:paginate total="${moduleParamCount ?: 0}"/>
  </div>
</div>
</body>
</html>