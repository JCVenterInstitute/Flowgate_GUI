<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.list.label" args="[entityName]"/></h2>

<f:table collection="${analysisServerList}" properties="['name', 'url', 'userName']"/>

<g:if test="${analysisServerCount > 10}">
  <ul class="pagination" id="pagination">
    <g:paginate total="${analysisServerCount ?: 0}" prev="chevron_left" next="chevron_right"/>
  </ul>
</g:if>

<div class="fixed-action-btn">
  <g:link class="btn-floating btn-large waves-effect waves-light tooltipped" action="create" data-tooltip="Add a new server" data-position="left">
    <i class="material-icons">add</i>
  </g:link>
</div>

<script>
  $(document).ready(function () {
    $("nav .nav-wrapper li").removeClass("active");
    $("nav #nav-analysis").addClass("active");

    updatePaginationToMaterial($('#pagination'));
  });
  document.addEventListener('DOMContentLoaded', function () {
    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
</body>
</html>
