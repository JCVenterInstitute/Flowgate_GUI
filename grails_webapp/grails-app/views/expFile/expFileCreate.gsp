<!DOCTYPE HTML>
<html>
<head>
  <meta name="layout" content="uploadLt"/>
  <g:set var="entityName" value="${message(code: 'uploadFcs.label', default: 'Upload FCS')}"/>
  <title><g:message code="default.uploadFcs.label" args="[entityName]" default="Upload FCS"/></title>
</head>

<body>
<div class="container">
  <h1 class="page-header">${experiment?.title}</h1>

  <h3 class="sub-header"><g:message code="default.fcsFile.create.label" default="Upload FCS-Files"/></h3>
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
    <g:hiddenField name="id" value="${this.expFile?.id}"/>
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
      <input type="file" name="actFcsFile" multiple accept=".fcs,.FCS"/>

      <p class="help-block">You can upload single or multiple .fcs file(s).</p>
    </div>
    <g:submitButton name="fcsUpload" class="save btn btn-primary" value="${message(code: 'default.button.annotate.label', default: 'Annotate')}"/>
    <a href="/flowgate/experiment/index?eId=${eId}" class="btn btn-warning">Back</a>
  </g:uploadForm>
</div>
</body>
</html>