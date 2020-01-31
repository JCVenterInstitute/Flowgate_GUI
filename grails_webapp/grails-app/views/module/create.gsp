<!DOCTYPE html>
<%@ page import="flowgate.AnalysisServer" defaultCodec="html" %>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.create.label" args="[entityName]"/></h2>
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
<g:form action="save">
  <f:with bean="module">
    <div class="col s4">
      <div class="input-field col s12">
        <g:select name="server" from="${flowgate.AnalysisServer.list()}" required="" optionKey="id" optionValue="name"/>
        <label>Server *</label>
      </div>
      <f:field property="title" label="Title" required="true"/>
      <f:field property="label" label="Label" required="false"/>
      <f:field property="name" label="Module or URN" required="true"/>
    </div>

    <div class="col s8">
      <div class="input-field">
        <label for="descript">Description</label>
        <g:textArea name="descript" class="materialize-textarea" style="height: 300px"/>
      </div>
    </div>
  </f:with>

  <div class="input-field col s12">
    <button type="submit" class="btn waves-effect waves-light">${message(code: 'default.button.create.label', default: 'Create')}</button>
    <g:link controller="module" action="index" class="btn-flat">Return to Module List</g:link>
  </div>
</g:form>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
    document.getElementById('descript').style.maxHeight = "300px";
    document.getElementById('descript').style.overflowY = "auto";
  });
</script>
</body>
</html>
