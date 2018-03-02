<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}" />
        <title><g:message code="default.edit.label" args="[entityName]" /></title>
    </head>
    <body>
        <a href="#edit-module" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
        <div class="nav" role="navigation">
            <ul>
                <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
                <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
                <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
            </ul>
        </div>
        <div id="edit-module" class="content scaffold-edit" role="main">
            <h1><g:message code="default.edit.label" args="[entityName]" /></h1>
            <g:if test="${flash.message}">
            <div class="message" role="status">${flash.message}</div>
            </g:if>
            <g:hasErrors bean="${this.module}">
            <ul class="errors" role="alert">
                <g:eachError bean="${this.module}" var="error">
                <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
                </g:eachError>
            </ul>
            </g:hasErrors>
            <g:form resource="${this.module}" method="PUT">
                <g:hiddenField name="version" value="${this.module?.version}" />
                <fieldset class="form">
                    %{--<f:all bean="module"/>--}%
                    %{--<br/>--}%
                    %{--<hr />--}%
                    %{--<br/>--}%
                    <f:with bean="module">
                        <f:field property="server" label="Server" required="true"/>
                        <f:field property="title" label="Module Title" required="true"/>
                        <f:field property="name" label="Module or URN" required="true"/>
                        %{--<div class="fieldcontain">
                            <label for="exp">Title</label>
                            <f:display id="exp" property="experiment.title" />
                            <g:hiddenField name="experimentId" value="${this.analysis.experiment}" />
                        </div>
                        <f:field property="analysisName" required="true"/>
                        <f:field property="analysisDescription" required="false"/>
                        <f:field property="analysisStatus" required="false"/>
                        <div class="fieldcontain">
                            <label for="jobNumber">Job Number</label>
                            <f:display id="jobNumber" property="jobNumber" />
                            <g:hiddenField name="jobNumber" value="${this.analysis.jobNumber}" />
                        </div>

                        <div class="fieldcontain">
                            <label for="usr">Sent by</label>
                            <f:display id="usr" property="user.username" />
                            <g:hiddenField name="userId" value="${this.analysis.user}" />
                        </div>

                        <div class="fieldcontain">
                            <label for="tstmp">Date Created</label>
                            <f:display id="tstmp" property="timestamp" />
                            <g:hiddenField name="timestamp" value="${this.analysis.timestamp}" />
                        </div>--}%
                    </f:with>
                    <g:render template="templates/moduleParamsTbl" model="[module: this?.module]" />
                </fieldset>
                <fieldset class="buttons">
                    <input class="save" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}" />
                </fieldset>
            </g:form>
        </div>
    </body>
</html>
