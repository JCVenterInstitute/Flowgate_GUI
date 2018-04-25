<div id="create-project" class="content scaffold-create" role="main">
  <h1><g:message code="default.create.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
  <g:hasErrors bean="${this.project}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.project}" var="error">
        <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>>
          <g:message error="${error}"/>
        </li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form action="save">
    <fieldset class="form">
      <f:with bean="project">
        <f:field property="title"/>
        <f:field property="description"/>
      </f:with>
    </fieldset>
    <fieldset class="text-center">
      <g:submitButton name="create" class="save btn btn-success"
                      value="${message(code: 'default.button.create.label', default: 'Create')}"/>
    </fieldset>
  </g:form>
</div>