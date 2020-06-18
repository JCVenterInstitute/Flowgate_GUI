<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<div id="list-user" class="content scaffold-list" role="main">
  <h1 class="text-center"><g:message code="default.list.label" args="[entityName]"/></h1>
  <div class="row">
    <div class="col-sm-offset-1 col-sm-10">
      <f:table collection="${userList}" properties="['username', 'enabled']"/>
    </div>
  </div>

  <div class="row">
    <div class="col-sm-offset-1">
      <div class="pagination">
        <g:paginate total="${userCount ?: 0}"/>
      </div>
    </div>
  </div>
</div>
</body>
</html>