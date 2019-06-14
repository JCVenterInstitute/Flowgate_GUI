<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>

<div class="nav" role="navigation"></div>

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="create-user" class="content scaffold-create" role="main">
      <ul class="breadcrumb">
        <li><g:link action="list"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
        <li class="active"><g:message code="default.create.label" args="[entityName]"/></li>
      </ul>

      <h1 class="page-header"><g:message code="default.create.label" args="[entityName]"/></h1>

      <g:if test="${flash.success}">
        <div class="alert alert-info">${flash.success}</div>
      </g:if>
      <g:hasErrors bean="${user}">
        <div class="alert alert-danger">
          <g:eachError var="err" bean="${user}">
            <li><g:message error="${err}"/></li>
          </g:eachError>
        </div>
      </g:hasErrors>

      <g:form resource="${user}" action="save"  class="form-horizontal col-md-6" onsubmit="return validatePass()">
        <div class="form-group">
          <label for="username" class="control-label col-sm-3">Username *</label>
          <div class="col-sm-9">
            <input type="text" name="username" required id="username" class="form-control" value="${user.username}">
          </div>
        </div>

        <div class="form-group">
          <label for="email" class="control-label col-sm-3">Email *</label>
          <div class="col-sm-9">
            <input type="text" name="email" id="email" class="form-control" value="${user.email}">
          </div>
        </div>

        <div class="form-group">
          <label for="affiliation" class="control-label col-sm-3">Affiliation *</label>
          <div class="col-sm-9">
            <input type="text" name="affiliation" required id="affiliation" class="form-control" value="${user.affiliation}">
          </div>
        </div>

        <div class="form-group">
          <label for="reason" class="control-label col-sm-3">Reason</label>
          <div class="col-sm-9">
            <input type="text" name="reason" id="reason" class="form-control" value="${user.reason}">
          </div>
        </div>

        <div class="form-group">
          <label for="enabled" class="control-label col-sm-3">Enabled</label>
          <div class="col-sm-9">
            <input type="hidden" name="_enabled" value="${user.enabled}">
            <input type="checkbox" name="enabled" id="enabled" class="form-control-static" style="margin: 0;" <g:if test="${user.enabled}">checked</g:if>>
          </div>
        </div>

        <div class="form-group">
          <label for="password" class="control-label col-sm-3">Password</label>
          <div class="col-sm-9">
            <input type="password" name="password" class="form-control" required>
          </div>
        </div>
        <div class="form-group" id="confirm-group">
          <label for="confirmpass" class="control-label col-sm-3">Confirm Password</label>
          <div class="col-sm-9">
            <input type="password" name="confirmpass" class="form-control" required>
            <div class="help-block"></div>
          </div>
        </div>

        <div class="form-group">
          <div class="col-sm-offset-3 col-sm-9">
            <g:link action="list" class="btn btn-default">Back</g:link>
            <button type="submit" class="btn btn-primary">Create User</button>
          </div>
        </div>
      </g:form>
    </div>
  </div>
</div>

<script>
  function validatePass() {
    var password = $("input[name='password']").val();
    var confirmPassword = $("input[name='confirmpass']").val();

    if (password != confirmPassword ) {
      $("#confirm-group").addClass("has-error");
      $("input[name='confirmpass']").next(".help-block").text("Password does not match!");
      return false;
    } else {
      return true;
    }
  }
</script>
</body>
</html>