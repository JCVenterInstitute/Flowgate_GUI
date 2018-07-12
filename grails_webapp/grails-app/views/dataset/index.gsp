<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<div class="container">
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
    <li class="active">List of Datasets for Analysis</li>
  </ul>

  <h1>List of Datasets for Analysis</h1>

  <div class="row">
    <div class="col-md-offset-9 col-md-3 mb-2" style="text-align: right;">
      <a href="${createLink(controller: 'dataset', action: 'create', params: [eId: experiment?.id])}"  class="btn btn-primary"><i class="fa fa-plus"></i>Create New Dataset</a>
      <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="btn btn-warning">Back</a>
    </div>
  </div>

  <table class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
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

  <div class="pagination">
    <g:paginate total="${datasetCount ?: 0}"/>
  </div>
</div>
</body>
</html>