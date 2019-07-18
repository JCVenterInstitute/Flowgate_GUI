<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<sec:ifAnyGranted roles="ROLE_Adminstrator">
  %{--<a href="#list-user" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%

  <div class="nav" role="navigation">
    %{--<ul>--}%
      %{--<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>--}%
      %{--<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]"/></g:link></li>--}%
    %{--</ul>--}%
  </div>
</sec:ifAnyGranted>
<div id="list-user" class="content scaffold-list" role="main">
  <h1 class="text-center"><g:message code="default.list.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
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