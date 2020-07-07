<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'dataset.label', default: 'Dataset')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
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

<h2><g:message code="default.edit.label" args="[entityName]"/></h2>

<g:hasErrors bean="${this.dataset}">
  <g:eachError var="err" bean="${this.dataset}">
    <script>
      document.addEventListener('DOMContentLoaded', function () {
        M.toast({
          html: '<span><g:message error="${err}"/></span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
          displayLength: Infinity,
          classes: 'red'
        });
      });
    </script>
  </g:eachError>
</g:hasErrors>

<g:form resource="${this.dataset}" params="[eId: experiment.id]" method="PUT">
  <g:hiddenField name="version" value="${this.dataset?.version}"/>

  <div class="row">
    <div class="input-field col s12">
      <input type="text" name="name" value="${this.dataset?.name}" required>
      <label for="name">Name</label>
    </div>

    <div class="input-field col s12">
      <textarea name="description" value="${this.description}" required class="materialize-textarea">${this.dataset?.description}</textarea>
      <label for="description">Description</label>
    </div>

    <div class="col s12">
      <g:render template="datasetTmpl/mdFilterPanel" model="[experiment: experiment]"/>

      <p><label><input type="checkbox" id="fcsSelectAll" class="filled-in">&nbsp;<span>Select all files</span></label></p>
    </div>

    <div class="col s12">
      <label for="fcsFiles">Files</label>

      <div id="fcsFiles" class="row">
        <g:render template="datasetTmpl/fcsFiles" model="[experiment: experiment, expFileCandidatesList: expFileCandidatesList, dataset: dataset]"/>
      </div>
    </div>

    <div class="input-field col s8">
      <button type="submit" class="btn waves-effect waves-light">${message(code: 'default.button.update.label', default: 'Update')}</button>
      <a href="${createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="btn-flat">Return to Datasets</a>
    </div>
  </div>
</g:form>

<script type="text/javascript">
  function setFilter() {
    var eId = ${experiment.id};
    var dsId = ${dataset.id};
    var filters = $(".fcs_filter:checked").map(function () {
      return {'key': this.getAttribute('key'), 'value': this.id};
    }).get();
    $.ajax({
      url: "${createLink(controller: 'dataset', action: 'axSetFilter')}",
      dataType: "json",
      data: {id: eId, filters: JSON.stringify(filters), dsId: dsId},
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
