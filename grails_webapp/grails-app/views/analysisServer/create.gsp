<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>

<div class="nav" role="navigation"></div>

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="create-analysisServer" class="content scaffold-create" role="main">
      <ul class="breadcrumb">
        <li><g:link action="index"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
        <li class="active"><g:message code="default.create.label" args="[entityName]" /></li>
      </ul>

      <h1 class="page-header"><g:message code="default.create.label" args="[entityName]"/></h1>

      <g:form resource="${analysisServer}" action="save" class="form-horizontal col-md-6">
        <g:hiddenField name="version" value="${this.analysisServer?.version}"/>
        <g:hiddenField name="user" value="${this.analysisServer?.user}"/>
        <g:if test="${flash.success}">
          <div class="alert alert-info">${flash.success}</div>
        </g:if>
        <g:hasErrors bean="${analysisServer}">
          <div class="alert alert-danger">
            <g:eachError var="err" bean="${analysisServer}">
              <li><g:message error="${err}" /></li>
            </g:eachError>
          </div>
        </g:hasErrors>

        <div class="form-group">
          <label for="name" class="control-label col-sm-3">Server Name *</label>
          <div class="col-sm-9">
            <input type="text" name="name" value="${analysisServer.name}" class="form-control" placeholder="Server Name" required>
          </div>
        </div>
        <div class="form-group">
          <label for="url" class="control-label col-sm-3">URL *</label>
          <div class="col-sm-9">
            <input type="text" name="url" value="${analysisServer.url}" class="form-control" placeholder="URL" required>
          </div>
        </div>
        <div class="form-group">
          <label for="userName" class="control-label col-sm-3">User Name *</label>
          <div class="col-sm-9">
            <input type="text" name="userName" value="${analysisServer.userName}" class="form-control" placeholder="User Name">
          </div>
        </div>
        <div class="form-group">
          <label for="userPw" class="control-label col-sm-3">User Password *</label>
          <div class="col-sm-9">
            <input type="password" name="userPw" value="${analysisServer.userPw}" class="form-control" placeholder="User Password">
          </div>
        </div>
        <div class="form-group">
          <div class="col-sm-offset-3 col-sm-9">
            <g:link controller="analysisServer" action="index" class="btn btn-default">Back</g:link>
            <button type="submit" class="btn btn-primary">Create Server</button>
          </div>
        </div>
      </g:form>
    </div>
  </div>
</div>
</body>
</html>
