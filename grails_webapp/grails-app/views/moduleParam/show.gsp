<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'moduleParam.label', default: 'ModuleParam')}"/>
  <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>
<div class="row">
  <h2><g:message code="default.show.label" args="[entityName]"/></h2>

  <f:with bean="moduleParam">
    <f:displayWidget property="module.label"/>
    <f:displayWidget property="pKey"/>
    <f:displayWidget property="pBasic"/>
    <f:displayWidget property="defaultVal"/>
    <f:displayWidget property="pType"/>
    <f:displayWidget property="pLabel"/>
    <f:displayWidget property="descr" label="Description"/>
    <div>
      <label>Example File</label>

      <p><a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}">${moduleParam?.exampleFile}</a></p>
    </div>
  </f:with>

  <g:link class="btn waves-effect waves-light" action="edit" resource="${this.moduleParam}">${message(code: 'default.button.edit.label', default: 'Edit')}</g:link>
  <a class="btn-flat modal-trigger" href="#delete-moduleparam-modal">${message(code: 'default.button.delete.label', default: 'Delete')}</a>
  <a href="${createLink(controller: 'module', action: 'edit', params: [id: moduleParam.module.id])}" class="btn-flat">Return to Module</a>
</div>

<div id="delete-moduleparam-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to delete module parameter</h4>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <g:link action="delete" resource="${this.moduleParam}" class="modal-close waves-effect waves-light btn-flat">Confirm</g:link>
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
