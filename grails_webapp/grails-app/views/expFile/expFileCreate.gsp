<!DOCTYPE HTML>
<html>
    <head>
        <meta name="layout" content="uploadLt" />
        <g:set var="entityName" value="${message(code: 'uploadFcs.label', default: 'Upload FCS')}" />
        <title><g:message code="default.uploadFcs.label" args="[entityName]" default="Upload FCS" /></title>
        <style>
            /*.uploader .fileinput {*/
            .uploadr .fileinput {
                height: 35px;
            }
        </style>
    </head>
    <body>

        %{--
        <a href="#create-project" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
        <div class="nav" role="navigation">
            <ul>
                <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
                <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
            </ul>
        </div>
        --}%
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
            %{--<g:form controller="expFile" action="uploadFcsFiles" name="uploadFcsFiles2">--}%
            <g:uploadForm controller="expFile" action="uploadFcsFiles" name="uploadFcsFiles">
            %{--<g:hiddenField name="version" value="${this.project?.version}" />--}%
            %{--<g:hiddenField name="isOpen" value="${flowgate?.Project?.list()?.size()<=0}" />--}%
            %{--<g:hiddenField name="inEditMode" value="${flowgate.Project.list().size()<=0}" />--}%
            %{--<g:hiddenField name="owners" value="${this?.project.owners}" />--}%
            %{--<g:hiddenField name="owners" value="${this.project.owners}" />--}%
            %{--<g:hiddenField name="members" value="${this.project.members}" />--}%
                <fieldset class="form">
                    %{--<f:all bean="project"/>--}%

                    %{--<f:with bean="project">--}%
                        %{--<f:field property="title"/>--}%
                        %{--<f:field property="description"/>--}%
                        %{--<f:field property="experiments"/>--}%
                    %{--<f:field property=""/>--}%
                    %{--<f:field property=""/>--}%
                    %{--<f:field property=""/>--}%
                    %{--</f:with>--}%

                    <g:hiddenField name="id" value="${this.expFile?.id}" />
                    <g:hiddenField name="version" value="${this.expFile?.version}" />
                    <input type="file" name="actFcsFile" multiple accept=".fcs,.FCS"/>

                </fieldset>
            %{--<fieldset class="buttons">--}%
                <fieldset class="text-center">
                    %{--<g:submitButton name="upload" class="save btn btn-success" value="${message(code: 'default.button.create.label', default: 'Upload')}" />--}%
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
            %{--<table>--}%
                %{----}%
            %{--</table>--}%
            %{--<uploadr:add name="myFirstUploadr" path="[:]/myFirstUploadr" maxSize="52428800" />--}%
            %{--<div class="row">
                <div class="col-sm-offset-4">
                    --}%%{--<span class="text-center">--}%%{--
                        <uploadr:add name="fcsUplds" path="/tmp/fcsUploads" allowedExtensions="fcs" downloadable="false" viewable="false" maxSize="0" controller="expFile" action="uploadFcsFiles2" />
                    --}%%{--</span>--}%%{--
                        <uploadr:onSuccess >
                            alert('upload success');
                            console.log('upload success');
                        </uploadr:onSuccess>

                </div>
            </div>

            <br/>
            <br/>
            <br/>
            <br/>
                <fieldset class="text-center">
                    <g:submitButton name="fcsUpload" class="save btn btn-success" value="${message(code: 'default.button.annotate.label', default: 'Annotate')}" />
                </fieldset>
            --}%
            %{--<uploadr:demo />--}%
            %{--</g:form>--}%
        </div>
    </body>
</html>