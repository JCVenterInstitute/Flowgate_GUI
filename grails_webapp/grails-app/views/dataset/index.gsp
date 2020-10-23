<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>

<div class="navigation nav-wrapper">
  <div class="col s12">
    <span class="breadcrumb dark" style="cursor: pointer;"><i id="breadcrumbs_unfold" class="material-icons" style="margin-left: -15px;">unfold_more</i></span>
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="#!" class="breadcrumb dark">Datasets</a>
  </div>
</div>

<h2>List of Datasets for Analysis</h2>

<g:if test="${datasetCount == 0}">
  <p>There is no dataset defined in this experiment. You need to <a
      href="${createLink(controller: 'dataset', action: 'create', params: [eId: experiment?.id])}">create a dataset</a> first.</p>
</g:if>
<g:else>
  <table class="highlight responsive-table">
    <thead>
    <tr>
      <g:sortableColumn class="text-center" property="name" title="Name"/>
      <g:sortableColumn class="text-center" property="description" title="Description"/>
      <g:sortableColumn class="text-center" property="files" title="Files"/>
    </tr>
    </thead>
    <tbody>
    <g:each in="${datasetList}" var="bean">
      <tr>
        <td><a href="edit/${bean?.id}">${bean?.name}</a></td>
        <td>${bean?.description}</td>
        <td><g:each in="${bean?.expFiles}" var="file">${file?.fileName}<br></g:each></td>
      </tr>
    </g:each>
    </tbody>
  </table>
</g:else>

<div class="pagination">
  <g:paginate total="${datasetCount ?: 0}"/>
</div>

<div class="fixed-action-btn">
  <a href="${createLink(controller: 'dataset', action: 'create', params: [eId: experiment?.id])}" class="btn-floating btn-large waves-effect waves-light tooltipped"
     data-tooltip="Create a new dataset" data-position="left"><i class="material-icons">add</i>
  </a>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
</body>
</html>
