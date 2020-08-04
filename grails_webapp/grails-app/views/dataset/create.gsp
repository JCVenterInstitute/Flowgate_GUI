<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
  <style>
  .hiddendiv.common {
    display: none;
  }
  </style>
</head>

<body>
<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark">Datasets</a>
    <a href="#!" class="breadcrumb dark">Create Dataset</a>
  </div>
</div>

<h2><g:message code="default.create.label" args="[entityName]"/></h2>

<g:if test="${expFileCandidatesList.size() == 0}">
  <p>There is no FCS file uploaded. You need to <a
      href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">upload a file</a> first.</p>
</g:if>
<g:else>
  <g:form resource="${this.dataset}" action="save" method="PUT">
    <g:hiddenField name="eId" value="${this.eId}"/>
    <g:hiddenField id="analyze" name="analyze" value="${false}"/>

    <div class="row">
      <div class="input-field col s12">
        <input type="text" name="name" value="${this.name}" required>
        <label for="name">Name</label>
      </div>

      <div class="input-field col s12">
        <textarea name="description" value="${this.description}" required class="materialize-textarea"></textarea>
        <label for="description">Description</label>
      </div>

      <div class="col s12">
        <g:render template="datasetTmpl/mdFilterPanel" model="[experiment: experiment]"/>

        <p><label><input type="checkbox" id="fcsSelectAll" class="filled-in">&nbsp;<span>Select all files</span></label></p>
      </div>

      <div class="col s12">
        <strong>Files</strong>

        <div id="fcsFiles" class="row">
          <g:render template="datasetTmpl/fcsFiles" model="[experiment: experiment, expFileCandidatesList: expFileCandidatesList, dataset: dataset]"/>
        </div>
      </div>

      <div class="input-field col s8">
        <button type="submit" class="btn waves-effect waves-light">${message(code: 'default.button.create.label', default: 'Create')}</button>
        <button type="submit" class="btn-flat" onclick="$('#analyze').val(${true});">Create and Analyze</button>
      </div>
    </div>
  </g:form>
</g:else>
<script type="text/javascript">
  function setFilter() {
    var eId = ${experiment.id};
    var filters = $(".fcs_filter:checked").map(function () {
      return {'key': this.getAttribute('key'), 'value': this.id};
    }).get();
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
