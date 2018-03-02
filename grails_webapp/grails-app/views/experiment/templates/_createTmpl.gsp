<div id="create-experiment" class="content scaffold-create" role="main">
%{--<h1><g:message code="default.create.label" args="[entityName]" /></h1>--}%
    %{--
    <g:if test="${flash.message}">
        <div class="message" role="status">${flash.message}</div>
    </g:if>
    <g:hasErrors bean="${this.experiment}">
        <ul class="errors" role="alert">
            <g:eachError bean="${this.experiment}" var="error">
                <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
            </g:eachError>
        </ul>
    </g:hasErrors>
    --}%
    <g:form action="save">

    %{--
    <fieldset class="form">
        <f:all bean="experiment"/>
    </fieldset>
    --}%

        %{--<g:hiddenField name="isOpen" value="false" />--}%
        %{--<g:hiddenField name="inEditMode" value="false" />--}%
        <g:hiddenField name="datasets" value="${null}" />
        <g:hiddenField name="experimentMeta" value="" />
        <g:hiddenField name="pId" value="${pId}" />
        <fieldset class="form">
            %{--<f:all bean="project"/>--}%
            <f:with bean="experiment">
                <f:field property="title"/>
                <f:field property="description"/>
                <f:field property="experimentHypothesis"/>
            %{--<f:field property=""/>--}%
            %{--<f:field property=""/>--}%
            %{--<f:field property=""/>--}%
            </f:with>
        </fieldset>
    %{--<fieldset class="buttons">--}%
        <fieldset class="">
            <g:submitButton name="create" class="save btn btn-success" value="${message(code: 'default.button.create.label', default: 'Create')}" />
        </fieldset>
    </g:form>
</div>