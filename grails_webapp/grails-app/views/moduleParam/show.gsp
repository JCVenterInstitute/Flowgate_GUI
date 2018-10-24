<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'moduleParam.label', default: 'ModuleParam')}"/>
  <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>
<div id="show-moduleParam" class="container" role="main">
  <h1 class="page-header"><g:message code="default.show.label" args="[entityName]"/></h1>

  <dl class="dl-horizontal">
    <f:with bean="moduleParam">
      <dt>Module Name:</dt>
      <dd><a href="${createLink(controller: 'module', action: 'edit', params: [id: moduleParam.module.id])}">${moduleParam.module.label}</a></dd>
      <f:displayWidget property="pKey"/>
      <f:displayWidget property="pBasic"/>
      <f:displayWidget property="defaultVal"/>
      <f:displayWidget property="pType"/>
      <f:displayWidget property="pLabel"/>
      <f:displayWidget property="descr" label="Description"/>
      <dt>Example File:</dt>
      <dd><a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}">${moduleParam?.exampleFile}</a></dd>
    </f:with>
    <dt></dt>
    <dd style="margin-top: 5px;">
      <g:form resource="${this.moduleParam}" method="DELETE">
        <fieldset class="buttons">
          <g:link class="edit btn btn-sm btn-primary" action="edit" resource="${this.moduleParam}"><g:message code="default.button.edit.label" default="Edit"/></g:link>
          <input class="delete btn btn-sm btn-danger" type="submit" value="${message(code: 'default.button.delete.label', default: 'Delete')}"
                 onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');"/>
        </fieldset>
      </g:form>
    </dd>
  </dl>
</div>
</body>
</html>
