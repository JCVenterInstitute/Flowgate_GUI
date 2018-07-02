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
  <g:form action="update" resource="${this.project}" method="POST" class="col-xs-6 col-sm-3">
    <g:hiddenField name="version" value="${this.project?.version}"/>
  %{--<g:hiddenField name="isOpen" value="${this.project?.isOpen}" />--}%
  %{--<g:hiddenField name="inEditMode" value="true" />--}%
  %{--<g:hiddenField name="owners" value="${this.project?.version}" />--}%
  %{--<g:hiddenField name="version" value="${this.project?.version}" />--}%
    <div class="form-group">
      <label for="title">Title *</label>
      <input type="text" class="form-control" id="title" name="title" placeholder="Title" required value="${project.title}">
    </div>

    <div class="form-group">
      <label for="description">Description *</label>
      <input type="text" class="form-control" id="description" name="description" placeholder="Description" required value="${project.description}">
    </div>
  %{--<fieldset class="buttons">--}%
  %{--<fieldset>--}%

    <button type="submit" class="btn btn-primary">${message(code: 'default.button.update.label', default: 'Update')}</button>
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: project?.id])}" type="submit" name="back" class="btn btn-warning">Back</a>
  %{--</fieldset>--}%
  </g:form>
</div>