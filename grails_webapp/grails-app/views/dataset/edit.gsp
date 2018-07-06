<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<div class="container">
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
    <li class="active">Edit Dataset</li>
  </ul>

  <h1><g:message code="default.edit.label" args="[entityName]"/></h1>
  <g:hasErrors bean="${this.dataset}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.dataset}" var="error">
        <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.dataset}" method="PUT" class="form-horizontal">
    <g:hiddenField name="version" value="${this.dataset?.version}"/>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Name</label>
      <div class="col-sm-11">
        <input type="text" class="form-control" id="name" name="name" placeholder="Name" value="${this.dataset?.name}">
      </div>
    </div>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Description</label>
      <div class="col-sm-11">
        <input type="text" class="form-control" id="description" name="description" placeholder="Description" value="${this.dataset?.description}">
      </div>
    </div>
    <div class="form-group">
      <div class="col-sm-offset-1 col-sm-11">
        <input class="btn btn-primary" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}"/>
        <a href="${createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="btn btn-warning">Back</a>
        <p class="help-block">Update Dataset Information</p>
      </div>
    </div>
  </g:form>
  <br>
  <g:form controller="dataset" action="dsExit" params="[experiment: experiment, dsId: dataset.id]" id="${experiment.id}">
    <div class="form-group">
      <div id="fcsCandidates" class="col-sm-5">
        <g:render template="datasetTmpl/fcsFileCandidates" model="[experiment: experiment, ds: dataset, dsId: dataset.id, expFileCandidatesList: expFileCandidatesList, pType: 'cand']"/>
      </div>
      <div class="col-sm-2">
        <div id="datasetBtnPnl" style="padding-top: 125%">
          <p><g:actionSubmit id="toDs" class="btn btn-default" style="width: 100%" action="assign" value="assign"/></p>
          <p><g:actionSubmit id="fromDs" class="btn btn-default" style="width: 100%" action="remove" value="remove"/></p>
          <p class="help-block">Assign/Remove FCS files</p>
        </div>
      </div>
      <div id="fcsAssigned" class="col-sm-5">
        <g:render template="datasetTmpl/fcsFileAssigned" model="[experiment: experiment, ds: dataset, dsId: dataset.id, expFileAssignedList: dataset.expFiles, pType: 'ass']"/>
      </div>
    </div>
  </g:form>
</div>

<script type="text/javascript">
  function selAllCandFcs(dsId) {
    var eId = ${experiment.id};
    var dsId = ${dataset.id};
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axSelAllCandFcs')}",
      dataType: "json",
      data: {id: eId, dsId: dsId},
      type: "get",
      success: function (data) {
        $("#fcsCandidates").html(data.fcsCandList);
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function deselAllCandFcs(dsId) {
    var eId = ${experiment.id};
    var dsId = ${dataset.id};
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axDeselAllCandFcs')}",
      dataType: "json",
      data: {id: eId, dsId: dsId},
      type: "get",
      success: function (data) {
        $("#fcsCandidates").html(data.fcsCandList);
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function selAllAssFcs(dsId) {
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axSelAllAssFcs')}",
      dataType: "json",
      data: {id: dsId},
      type: "get",
      success: function (data) {
        $("#fcsAssigned").html(data.fcsAssList);
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function deselAllAssFcs(dsId) {
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axDeselAllAssFcs')}",
      dataType: "json",
      // data: {id: eId, dsId: dsId},
      data: {id: dsId},
      type: "get",
      success: function (data) {
        $("#fcsAssigned").html(data.fcsAssList);
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function dsSelChange(dsId) {
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axDsChange')}",
      dataType: "json",
      type: "get",
      %{--data: {eId: ${params?.eId}, modId: moduleId},--}%
      data: {id: dsId},
      success: function (data) {
        // alert("ds Changed!");
        $("#metaData").html(data.metaData);
        $("#dsPanel").html(data.dsPanel);
        // $("#fcsCandidates").html(data.fcsCandidates);
        $("#fcsAssigned").html(data.fcsAssigned);
        console.log('ds changed');
      },
      error: function (request, status, error) {
        console.log('E: ' + error);
        // alert("ds Change error");
      },
      complete: function () {
        // alert("ds Change completed");
        console.log('ajax ds change completed');
      }
    });
  }
</script>
</body>
</html>
