<div id="create-project" class="content scaffold-create" role="main">
  <h1 class="page-header"><g:message code="default.create.label" args="[entityName]"/></h1>
  <g:form action="save" class="col-xs-12 col-sm-6">
    <f:with bean="project">
      <f:field property="title"/>
      <f:field property="description"/>
    </f:with>
    <g:submitButton name="create" class="btn btn-primary" value="${message(code: 'default.button.create.label', default: 'Create')}"/>
    <a href="${createLink(controller: 'project', action:'list')}" class="btn btn-warning">Back</a>
  </g:form>
</div>