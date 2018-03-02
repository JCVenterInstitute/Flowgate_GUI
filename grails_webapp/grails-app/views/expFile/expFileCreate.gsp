<!DOCTYPE HTML>
<html>
    <head>
        <meta name="layout" content="uploadLt" />
        <g:set var="entityName" value="${message(code: 'uploadFcs.label', default: 'Upload FCS')}" />
        <title><g:message code="default.uploadFcs.label" args="[entityName]" default="Upload FCS" /></title>
    </head>
    <body>
        <div id="create-project" class="content scaffold-create" role="main">
            <h3 class="text-center"><g:message code="default.fcsFile.create.label" default="Upload FCS-Files" /></h3>
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
                <fieldset class="form">
                    <g:hiddenField name="id" value="${this.expFile?.id}" />
                    <g:hiddenField name="version" value="${this.expFile?.version}" />
                    <input type="file" name="actFcsFile" multiple accept=".fcs,.FCS"/>
                </fieldset>
                <fieldset class="text-center">
                    <g:submitButton name="fcsUpload" class="save btn btn-success" value="${message(code: 'default.button.annotate.label', default: 'Annotate')}" />
                </fieldset>
            </g:uploadForm>
            <br/>
            <br/>
            <br/>

            <g:if test="${flash.errMsg && flash.errMsg != ''}">
                <div class="alert">
                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
                    <strong>Error</strong> ${flash.errMsg}
                </div>
            </g:if>
        </div>
    </body>
</html>