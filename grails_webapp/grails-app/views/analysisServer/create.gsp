<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.create.label" args="[entityName]"/></h2>

<div class="row">
  <g:form resource="${analysisServer}" action="save" class="col s6">
    <g:hiddenField name="version" value="${this.analysisServer?.version}"/>
    <g:hiddenField name="user" value="${this.analysisServer?.user}"/>
    <g:hasErrors bean="${analysisServer}">
      <div class="alert alert-danger">
        <g:eachError var="err" bean="${analysisServer}">
          <li><g:message error="${err}"/></li>
        </g:eachError>
      </div>
    </g:hasErrors>


    <f:with bean="analysisServer">
      <div class="row">
        <f:field property="name" label="Server Name" required="true"/>
        <f:field property="url" required="true"/>

        <div class="input-field col s12">
          <select name="platform">
            <option value="1">GenePattern</option>
            <option value="2">ImmportGalaxy</option>
          </select>
          <label>Select a Platform</label>
        </div>

        <f:field property="userName" label="Username" required="true"/>
        <f:field property="userPw" label="Password" password="true" required="true"/>
      </div>
    </f:with>

    <div class="row">
      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Create Server</button>
        <g:link controller="analysisServer" action="index" class="btn-flat">Return to Analysis Server List</g:link>
      </div>
    </div>
  </g:form>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function() {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
  });
</script>
</body>
</html>
