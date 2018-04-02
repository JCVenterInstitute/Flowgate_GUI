%{--
    <a href="#edit-project" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
    <div class="nav" role="navigation">
        <ul>
            <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
            <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
            <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
        </ul>
    </div>
--}%
<div id="edit-project" class="content scaffold-edit" role="main">
%{--<h1><g:message code="default.edit.label" args="[entityName]" /></h1>--}%
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
  <g:hasErrors bean="${this.project}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.project}" var="error">
        <li<g:if
                test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message
             error="${error}"/></li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.project}" method="PUT">
    <g:hiddenField name="version" value="${this.project?.version}"/>
  %{--<g:hiddenField name="isOpen" value="${this.project?.isOpen}" />--}%
  %{--<g:hiddenField name="inEditMode" value="true" />--}%
  %{--<g:hiddenField name="owners" value="${this.project?.version}" />--}%
  %{--<g:hiddenField name="version" value="${this.project?.version}" />--}%
    <fieldset class="form">
      %{--<f:all bean="project"/>--}%
      <f:with bean="project">
        <f:field property="title"/>
        <f:field property="description"/>
      %{--<f:field property=""/>--}%
      %{--<f:field property=""/>--}%
      </f:with>
    </fieldset>
  %{--<fieldset class="buttons">--}%
  %{--<fieldset>--}%
    <input class="save btn btn-success" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}"/>
  %{--</fieldset>--}%
  </g:form>
</div>