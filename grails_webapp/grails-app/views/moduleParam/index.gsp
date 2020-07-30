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

  <g:if test="${moduleParamCount > 10}">
    <ul class="pagination" id="pagination">
      <g:paginate total="${moduleParamCount ?: 0}" prev="chevron_left" next="chevron_right"/>
    </ul>
  </g:if>
</div>

<script>
  $(document).ready(function () {
    updatePaginationToMaterial($('#pagination'));
  });
</script>
</body>
</html>
