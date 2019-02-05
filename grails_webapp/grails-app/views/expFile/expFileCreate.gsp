<!DOCTYPE HTML>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'uploadFcs.label', default: 'Upload FCS')}"/>
  <title><g:message code="default.uploadFcs.label" args="[entityName]" default="Upload FCS"/></title>
</head>

<body>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>
</content>
<content tag="pgContent">
<h1 class="page-header">${experiment?.title}</h1>

<h3 class="sub-header"><g:message code="default.fcsFile.create.label" default="Upload FCS Files"/></h3>
<g:if test="${flash.message}">
  <div class="message" role="status">${flash.message}</div>
</g:if>
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
  <div class="form-group">
    <input id="uploadFile" type="file" name="actFcsFile" multiple/>

    <p class="help-block">You can upload single or multiple .fcs file(s).</p>
  </div>
  <p>Or you can drag and drop your files in the box below to upload (Max size for each file is 1GB)</p>
  <div id="dropzone" class="well">Drop files here</div>
  <div id="files" class="files"></div>
  <div class="row mt-3">
    <div class="col-md-12">
      <input type="button" class="btn btn-primary start" id="uploadFilesBtn" value="${message(code: 'default.button.annotate.label', default: 'Upload')}"/>
      <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: eId])}" class="btn btn-warning">Back</a>
    </div>
  </div>
</g:uploadForm>
</content>
<content tag="pgAssets">
  <asset:javascript src="jquery.ui.widget.js" />
  <asset:javascript src="fileupload/jquery.fileupload.js"/>
  <asset:javascript src="fileupload/jquery.fileupload-process.js"/>
  <asset:javascript src="fileupload/jquery.fileupload-validate.js"/>
  <asset:javascript src="fileupload/jquery.fileupload-ui.js"/>
  <asset:javascript src="fileupload/jquery.fileupload-main.js"/>
</content>
</body>
</html>