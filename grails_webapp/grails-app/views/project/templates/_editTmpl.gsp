<div id="edit-project" class="content scaffold-edit" role="main">
  <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>
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
  <g:form action="update" resource="${this.project}" method="POST" class="col-xs-12 col-sm-6">
    <g:hiddenField name="version" value="${this.project?.version}"/>
    <div class="form-group">
      <label for="title">Title *</label>
      <textarea class="form-control" id="title" name="title" placeholder="Title" rows="5" required>${project.title}</textarea>
    </div>

    <div class="form-group">
      <label for="description">Description *</label>
      <textarea class="form-control" id="description" name="description" placeholder="Description" rows="10" required>${project.description}</textarea>
    </div>

    <button type="submit" class="btn btn-primary">${message(code: 'default.button.update.label', default: 'Update')}</button>
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: project?.id])}" type="submit" name="back" class="btn btn-warning">Back</a>
  </g:form>
</div>