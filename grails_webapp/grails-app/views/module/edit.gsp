<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
  <style>
  .hiddendiv.common {
    display: none;
  }
  </style>
</head>

<body>

<h2 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h2>
<g:hasErrors bean="${this.module}">
  <g:eachError var="err" bean="${this.module}">
    <script>
      document.addEventListener('DOMContentLoaded', function () {
        M.toast({
          html: '<span><g:message error="${err}"/></span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
          displayLength: Infinity,
          classes: 'red'
        });
      });
    </script>
  </g:eachError>
</g:hasErrors>
<g:form resource="${this.module}" method="PUT" class="row">
  <g:hiddenField name="version" value="${this.module?.version}"/>
  <f:with bean="module">
    <div class="col s4">
      <div class="row">
        <f:field property="server" label="Server" required="true"/>
        <f:field property="label" label="Label" required="false"/>
        <f:field property="title" label="Title" required="true"/>
        <f:field property="name" label="Module or URN" required="true"/>
      </div>
    </div>

    <div class="col s8">
      <f:field property="descript" label="Module Description" required="false" rows="5"/>
    </div>
  </f:with>

  <g:if test="${module.moduleParams.size() > 0}">
    <div class="col s12">
      <h5>Module Parameters</h5>
      <f:table collection="${module.moduleParams}" properties="['pKey', 'pLabel', 'defaultVal', 'pBasic', 'pType', 'descr', 'exampleFile']"/>
    </div>
  </g:if>

  <div class="input-field col s12">
    <button class="btn waves-effect waves-light" type="submit">${message(code: 'default.button.update.label', default: 'Update')}</button>
    <a href="${createLink(controller: 'moduleParam', action: 'create', params: ['module.id': module.id])}" class="btn-flat">Add Module Parameter</a>
    <g:link controller="module" action="index" class="btn-flat">Return to Module List</g:link>
  </div>
</g:form>
<script>
  document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('descript').style.maxHeight = "300px";
    document.getElementById('descript').style.overflowY = "auto";
  });
</script>
</body>
</html>
