<!DOCTYPE HTML>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'uploadFcs.label', default: 'Upload FCS')}"/>
  <title><g:message code="default.uploadFcs.label" args="[entityName]" default="Upload FCS"/></title>
</head>

<body>
<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="#!" class="breadcrumb dark">Upload FCS Files</a>
  </div>
</div>

<h2><g:message code="default.fcsFile.create.label" default="Upload FCS Files"/></h2>
<g:hasErrors bean="${this.expFile}">
  <ul class="errors" role="alert">
    <g:eachError bean="${this.expFile}" var="error">
      <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
    </g:eachError>
  </ul>
</g:hasErrors>
<g:uploadForm controller="expFile" action="uploadFcsFiles" name="uploadFcsFiles">
  <g:hiddenField name="eId" value="${eId}"/>
  <g:hiddenField name="version" value="${this.expFile?.version}"/>
  <g:if test="${flash.errMsg && flash.errMsg != ''}">
    <div class="form-group">
      <div class="alert alert-danger alert-dismissible">
        <a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>
        <strong>Error:</strong> ${flash.errMsg}
      </div>
    </div>
  </g:if>
  <div class="col s12">
    <div class="file-field input-field">
      <div class="btn">
        <span>Choose File(s)</span>
        <input id="uploadFile" type="file" name="actFcsFile" multiple/>
      </div>

      <div class="file-path-wrapper">
        <input class="file-path validate" type="text" placeholder="Upload one or more .fcs file(s)">
      </div>
    </div>

    <div id="dropzone" class="card-panel hoverable">Drop files here</div>

    <blockquote>You can select single or multiple .fcs file(s).<br>
      You can drag and drop your files in the box above to upload (Max size for each file is 1GB)</blockquote>

    <ul id="files" class="collection files"></ul>

    <div class="input-field">
      <button type="button" class="btn waves-effect waves-light" id="uploadFilesBtn">${message(code: 'default.button.annotate.label', default: 'Upload')}</button>
      <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="btn-flat">Cancel</a>
    </div>
  </div>
</g:uploadForm>

<asset:javascript src="jquery.ui.widget.js"/>
<asset:javascript src="fileupload/jquery.fileupload.js"/>
<asset:javascript src="fileupload/jquery.fileupload-process.js"/>
<asset:javascript src="fileupload/jquery.fileupload-validate.js"/>
<asset:javascript src="fileupload/jquery.fileupload-ui.js"/>
<asset:javascript src="fileupload/jquery.fileupload-main.js"/>
</body>
</html>