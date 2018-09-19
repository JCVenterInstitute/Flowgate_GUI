<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>

<div class="container">
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
    <li><a href="${createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}">List of Datasets for Analysis</a></li>
    <li class="active">Create Dataset</li>
  </ul>

  <h1><g:message code="default.create.label" args="[entityName]"/></h1>
  <g:if test="${flash.error}">
    <div class="row justify-content-center ">
      <div class="alert alert-danger text-center" role="alert">${flash.error}</div>
    </div>
  </g:if>
  <g:form resource="${this.dataset}" action="save" method="PUT" class="form-horizontal">
    <g:hiddenField name="eId" value="${this.eId}"/>
    <g:hiddenField id="analyze" name="analyze" value="${false}"/>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Name *</label>
      <div class="col-sm-11">
        <input type="text" class="form-control" id="name" name="name" placeholder="Name" value="${this.name}">
      </div>
    </div>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Description</label>
      <div class="col-sm-11">
        <input type="text" class="form-control" id="description" name="description" placeholder="Description" value="${this.description}">
      </div>
    </div>
    <div class="form-group">
      <div class="col-sm-offset-1 col-sm-11">
        <div class="col-sm-12">
          <a class="s_h_filter" style="color:#333333; cursor: pointer;">
            <p><strong>Filter</strong>
            <i class="fa fa-angle-up"></i></p>
          </a>
          <div id="metaData">
            <g:render template="datasetTmpl/mdFilterPanel" model="[experiment: experiment]"/>
          </div>
        </div>
        <div class="col-sm-2">
          <input type="checkbox" id="fcsSelectAll">&nbsp;<span>Select All</span>
        </div>
      </div>
    </div>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Files</label>
      <div class="col-sm-11">
        <g:render template="datasetTmpl/fcsFiles" model="[experiment: experiment, expFileCandidatesList: expFileCandidatesList, dataset: dataset]"/>
      </div>
    </div>
    <div class="form-group">
      <div class="col-sm-offset-1 col-sm-11">
        <input class="btn btn-primary" type="submit" value="${message(code: 'default.button.create.label', default: 'Create')}"/>
        <input class="btn btn-info" type="submit" onclick="$('#analyze').val(${true});" value="Create and Analyze"/>
        <a href="${createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="btn btn-warning">Back</a>
      </div>
    </div>
  </g:form>
</div>
<script type="text/javascript">
  function setFilter() {
    var eId = ${experiment.id};
    var filters = $(".fcs_filter:checked").map(function() { return this.id; }).get();
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axSetFilter')}",
      dataType: "json",
      data: {id: eId, filters: JSON.stringify(filters)},
      type: "get",
      success: function (data) {
        $("#fcsFiles").html(data.fcsList);
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }
</script>
</body>
</html>
