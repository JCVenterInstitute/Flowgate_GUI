<div id="create-project" class="content scaffold-create" role="main">
  <h1 class="page-header"><g:message code="default.create.label" args="[entityName]"/></h1>
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
  <g:form action="save" class="col-xs-6 col-sm-3">
    %{-- --}%
    <div class="form-group">
      <label for="title">Title *</label>
      <input type="text" class="form-control" id="title" name="title" placeholder="Title" required>
    </div>

    <div class="form-group">
      <label for="description">Description *</label>
      <input type="text" class="form-control" id="description" name="description" placeholder="Description" required>
    </div>
  %{-- --}%

    <f:with bean="project">
      <f:field property="title"/>
      <f:field property="description"/>
    </f:with>

    <g:submitButton name="create" class="btn btn-primary" value="${message(code: 'default.button.create.label', default: 'Create')}"/>
    <a href="/flowgate/project/list" class="btn btn-warning">Back</a>
  </g:form>
</div>