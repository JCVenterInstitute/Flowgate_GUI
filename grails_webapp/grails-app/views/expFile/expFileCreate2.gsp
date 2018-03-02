<!DOCTYPE HTML>
<html>
    <head>
        <asset:javascript src="uploadr.manifest.js" />
        <asset:javascript src="uploadr.demo.manifest.js" />
        <asset:stylesheet href="uploadr.manifest.css" />
        <asset:stylesheet href="uploadr.demo.manifest.css" />
    </head>
    <body>
        <div id="create-project" class="content scaffold-create" role="main">
            <h1><g:message code="default.fcsFile.create.label" default="Upload FCS-Files" /></h1>
            <g:if test="${flash.message}">
                <div class="message" role="status">${flash.message}</div>
            </g:if>
            %{--TODO check hasErrors functionality--}%
            %{--
            <g:hasErrors bean="${this.expFile}">
                <ul class="errors" role="alert">
                    <g:eachError bean="${this.expFile}" var="error">
                        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
                    </g:eachError>
                </ul>
            </g:hasErrors>
            --}%
            %{--
            <g:uploadForm action="uploadFcsFile" name="uploadFcsFile" >
                <fieldset class="form">
                    <g:hiddenField name="id" value="${this.fcsFile?.id}" />
                    <g:hiddenField name="version" value="${this.fcsFile?.version}" />
                    <input type="file" id="fcsFiles" name="fcsFiles[]" multiple />
                </fieldset>
                <fieldset class="text-center">
                    <g:submitButton name="upload" class="save btn btn-success" value="${message(code: 'default.button.fcsFile.create.label', default: 'Upload')}" />
                </fieldset>
            </g:uploadForm>
            --}%
            <uploadr:demo />
        </div>
    </body>
</html>
