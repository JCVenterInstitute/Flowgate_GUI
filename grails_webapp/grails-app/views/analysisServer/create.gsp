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

    <div class="row">
      <div class="input-field col s12">
        <input type="text" name="name" value="${analysisServer.name}" required>
        <label for="name">Server Name</label>
      </div>

      <div class="input-field col s12">
        <input type="text" name="url" value="${analysisServer.url}" required>
        <label for="url">URL</label>
      </div>

      <div class="input-field col s12">
        <input type="text" name="userName" value="${analysisServer.userName}" required>
        <label for="userName">Username</label>
      </div>

      <div class="input-field col s12">
        <input type="password" name="userPw" value="${analysisServer.userPw}" required>
        <label for="userPw">Password</label>
      </div>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Create Server</button>
        <g:link controller="analysisServer" action="index" class="btn-flat">Return to Analysis List</g:link>
      </div>
    </div>
  </g:form>
</div>
</body>
</html>
