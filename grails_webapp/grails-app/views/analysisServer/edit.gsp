<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>

<div class="nav" role="navigation"></div>
<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="edit-analysisServer" class="content scaffold-edit" role="main">
      <ul class="breadcrumb">
        <li><g:link action="index"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
        <li class="active"><g:message code="default.edit.label" args="[entityName]" /></li>
      </ul>

      <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>

      <g:form resource="${analysisServer}" method="PUT" class="form-horizontal col-md-6">
        <g:hiddenField name="version" value="${this.analysisServer?.version}"/>
        <g:if test="${flash.success}">
          <div class="alert alert-info">${flash.success}</div>
        </g:if>
        <g:hasErrors bean="${user}">
          <div class="alert alert-danger">
            <g:eachError var="err" bean="${user}">
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
          <div class="col-sm-offset-3 col-sm-9">
            <g:link controller="analysisServer" action="index" class="btn btn-default">Back</g:link>
            <g:link action="delete" resource="${analysisServer}" onclick="return confirm('Are you sure to delete server?');" class="btn btn-default">Delete</g:link>
            <button type="submit" class="btn btn-primary">Update Server</button>
          </div>
        </div>
      </g:form>

      <g:form resource="${analysisServer}" method="PUT" action="updatePassword" class="form-horizontal col-md-6" onsubmit="return validatePass()">
        <g:if test="${flash.passSuccess}">
          <div class="alert alert-info">${flash.passSuccess}</div>
        </g:if>
        <g:if test="${flash.passError}">
          <div class="alert alert-danger">${flash.passError}</div>
        </g:if>

        <div class="form-group">
          <label for="oldpass" class="control-label col-sm-4">Old Password</label>
          <div class="col-sm-8">
%{--            <input type="password" id="oldpass" name="oldpass" class="form-control" placeholder="Old Password" required>--}%
            <input type="password" id="oldpass" name="oldpass" class="form-control" placeholder="Old Password" >
          </div>
        </div>
        <div class="form-group">
          <label for="newpass" class="control-label col-sm-4">New Password</label>
          <div class="col-sm-8">
            <input type="password" name="newpass" class="form-control" placeholder="New Password" required>
          </div>
        </div>
        <div class="form-group">
          <div class="col-sm-offset-4 col-sm-8">
            <button type="submit" class="btn btn-primary">Update Password</button>
          </div>
        </div>
      </g:form>

    </div>
  </div>
</div>
</body>
</html>
