<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.list.label" args="[entityName]"/></h2>

<f:table collection="${moduleList}" properties="${['title', 'label', 'name', 'server']}"/>
<g:if test="${moduleCount > 10}">
  <ul class="pagination" id="pagination">
    <g:paginate total="${moduleCount ?: 0}" prev="chevron_left" next="chevron_right"/>
  </ul>
</g:if>

<div class="fixed-action-btn">
  <g:link class="btn-floating btn-large waves-effect waves-light tooltipped" action="create" data-tooltip="Add a new module" data-position="left">
    <i class="material-icons">add</i>
  </g:link>
</div>

<script>
  $(document).ready(function () {
    $("nav .nav-wrapper li").removeClass("active");
    $("nav #nav-modules").addClass("active");

    updatePaginationToMaterial($('#pagination'));
  });
  document.addEventListener('DOMContentLoaded', function () {
    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
</body>
</html>
