<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Transformation Parameters')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.list.label" args="[entityName]"/></h2>

<f:table collection="${transformationParameterList}" properties="${['transformName', 'transformType', 'parameterValues']}"/>
<g:if test="${transformationParameterCount > 10}">
  <ul class="pagination" id="pagination">
    <g:paginate total="${transformationParameterCount ?: 0}" prev="chevron_left" next="chevron_right"/>
  </ul>
</g:if>

<div class="fixed-action-btn">
  <g:link class="btn-floating btn-large waves-effect waves-light tooltipped" action="create" data-tooltip="Add a new parameter" data-position="left">
    <i class="material-icons">add</i>
  </g:link>
</div>

<script>
  $(document).ready(function () {
    $("nav .nav-wrapper li").removeClass("active");
    $("nav #nav-transformation").addClass("active");

    updatePaginationToMaterial($('#pagination'));
  });
  document.addEventListener('DOMContentLoaded', function () {
    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
</body>
</html>
