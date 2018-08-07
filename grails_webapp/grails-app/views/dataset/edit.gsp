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
  <g:form resource="${this.dataset}" params="[eId: experiment.id]" method="PUT" class="form-horizontal">
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
        <input type="checkbox" id="fcsSelectAll">&nbsp;<span>Select All</span>
      </div>
    </div>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Files</label>
      <div class="col-sm-11">
        <g:set var="split" value="${Math.ceil(expFileCandidatesList?.size()/3).toInteger()}" />
        <g:set var="counter" value="${0}"/>
        <g:each in="${expFileCandidatesList?.sort { it.fileName }}" var="expFile" status="index">
          <g:set var="counter" value="${counter + 1}"/>
          <g:if test="${counter % split == 1}"><div class="col-sm-4"></g:if>
          <p><g:checkBox class="fcs_files" name="file_${expFile.id}" checked="${expFile.id in dataset.expFiles.id}"/>&nbsp;<span>${expFile.fileName}</span></p>
          <g:if test="${counter % split == 0 || index+1 == expFileCandidatesList?.size()}"></div></g:if>
        </g:each>
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
</div>

<script type="text/javascript">
  $(document).ready(function () {
    $("#fcsSelectAll").click(function () {
      $(".fcs_files").prop('checked', $(this).prop('checked'));
    });
    $('.fcs_files').change(function(){ //".checkbox" change
      //uncheck "select all", if one of the listed checkbox item is unchecked
      if(this.checked == false){ //if this item is unchecked
        $("#fcsSelectAll")[0].checked = false; //change "select all" checked status to false
      }

      //check "select all" if all checkbox items are checked
      if ($('.fcs_files:checked').length == $('.fcs_files').length ){
        $("#fcsSelectAll")[0].checked = true; //change "select all" checked status to true
      }
    });
  });
</script>
</body>
</html>
