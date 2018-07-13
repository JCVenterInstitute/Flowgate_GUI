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
    <li class="active">Create Dataset</li>
  </ul>

  <h1><g:message code="default.create.label" args="[entityName]"/></h1>
  <g:hasErrors bean="${this.dataset}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.dataset}" var="error">
        <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.dataset}" action="save" method="PUT" class="form-horizontal">
    <g:hiddenField name="eId" value="${this.eId}"/>
    <div class="form-group">
      <label class="col-sm-1 control-label" for="description">Name</label>
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
      <label class="col-sm-1 control-label" for="description">Files</label>
      <div class="col-sm-11">
        <g:each in="${expFileCandidatesList?.sort { it.fileName }}" var="expFile">
          <p><g:checkBox name="file_${expFile.id}" checked="${expFile.id in session.selectedFiles}"/>&nbsp;<span>${expFile.fileName}</span></p>
        </g:each>
      </div>
    </div>
    <div class="form-group">
      <div class="col-sm-offset-1 col-sm-11">
        <input class="btn btn-primary" type="submit" value="${message(code: 'default.button.create.label', default: 'Create')}"/>
        <a href="${createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="btn btn-warning">Back</a>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
