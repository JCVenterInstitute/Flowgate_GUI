<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.show.label" args="[entityName]"/></h2>

<f:with bean="module">
  <div class="col s4">
    <f:field property="server.name" label="Server"/>
    <f:field property="title" label="Title"/>
    <f:field property="label" label="Label"/>
    <f:field property="name" label="Module or URN"/>
  </div>

  <div class="col s8">
    <f:field property="descript" label="Description"/>
  </div>
</f:with>

<div class="input-field col s12">
  <g:link class="btn waves-effect waves-light" action="edit" resource="${this.module}">${message(code: 'default.button.edit.label', default: 'Edit')}</g:link>
  <a class="btn-flat modal-trigger" href="#delete-module-modal">${message(code: 'default.button.delete.label', default: 'Delete')}</a>
  <g:link controller="module" action="index" class="btn-flat">Return to Module List</g:link>
</div>

<div id="delete-module-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to delete module</h4>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <g:link action="delete" resource="${this.module}" class="modal-close waves-effect waves-light btn-flat">Confirm</g:link>
  </div>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems);
  });
</script>
</body>
</html>
