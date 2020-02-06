<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'moduleParam.label', default: 'ModuleParam')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.create.label" args="[entityName]"/></h2>
<g:hasErrors bean="${this.moduleParam}">
  <g:eachError var="err" bean="${this.moduleParam}">
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
<div class="row">
  <g:form action="save" class="col s6">
    <div class="row">
      <f:with bean="moduleParam">
        <g:hiddenField name="module.id" value="${moduleParam.module.id}"/>
        <div class="input-field col s12">
          <input type="text" value="${moduleParam.module.title}" disabled>
          <label>Module *</label>
        </div>
        <f:field property="pKey"/>
        <div class="col s12">
          <p>
            <label>
              <f:input property='pBasic'/>
              <span>Basic Parameter</span>
            </label>
          </p>
        </div>
        <f:field property="defaultVal"/>
        <f:field property="pType"/>
        <f:field property="pLabel"/>
        <f:field property="pOrder"/>
        <f:field property="descr"/>
        <f:field property="exampleFile"/>
      </f:with>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">${message(code: 'default.button.create.label', default: 'Create')}</button>
        <a href="${createLink(controller: 'module', action: 'edit', params: [id: moduleParam.module.id])}" class="btn-flat">Return to Module</a>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
