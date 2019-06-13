<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<div class="nav" role="navigation"></div>
<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="edit-user" class="content scaffold-edit" role="main">
      <ul class="breadcrumb">
        <li><g:link controller="user" action="list">Users</g:link></li>
        <li class="active"><g:message code="default.edit.label" args="[entityName]" /></li>
      </ul>

      <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>

      <g:form resource="${user}" method="PUT" class="form-horizontal col-md-6">
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
          <label for="username" class="control-label col-sm-3">Username *</label>
          <div class="col-sm-9">
            <input type="text" name="username" value="${user.username}" class="form-control" placeholder="Username" required>
          </div>
        </div>
        <div class="form-group">
          <label for="email" class="control-label col-sm-3">Email *</label>
          <div class="col-sm-9">
            <input type="text" name="email" value="${user.email}" class="form-control" placeholder="Email" required>
          </div>
        </div>
        <div class="form-group">
          <label for="affiliation" class="control-label col-sm-3">Affiliation</label>
          <div class="col-sm-9">
            <input type="text" name="affiliation" value="${user.affiliation}" class="form-control" placeholder="Affiliation">
          </div>
        </div>
        <g:if test="${!user.authorities.authority.contains("ROLE_Admin") && !user.authorities.authority.contains("ROLE_Administrator")}" >
          <div class="form-group">
            <label for="enabled" class="control-label col-sm-3">Active</label>
            <div class="col-sm-9">
              <input type="checkbox" id="enabled" class="form-control-static" style="margin: 0;" <g:if test="${user.enabled}">checked</g:if> />
              <input type="hidden" name="enabled" value="${user.enabled}"/>
            </div>
          </div>
        </g:if>
        <div class="form-group">
          <div class="col-sm-offset-3 col-sm-9">
            <g:link controller="user" action="list" class="btn btn-default">Back</g:link>
            <button type="submit" class="btn btn-primary">Update User</button>
          </div>
        </div>
      </g:form>

      <g:form resource="${user}" method="PUT" action="updatePassword" class="form-horizontal col-md-6" onsubmit="return validatePass()">
        <g:if test="${flash.passSuccess}">
          <div class="alert alert-info">${flash.passSuccess}</div>
        </g:if>
        <g:if test="${flash.passError}">
          <div class="alert alert-danger">${flash.passError}</div>
        </g:if>

        <div class="form-group">
          <label for="oldpass" class="control-label col-sm-4">Old Password</label>
          <div class="col-sm-8">
            <input type="password" name="oldpass" class="form-control" placeholder="Old Password" required>
          </div>
        </div>
        <div class="form-group">
          <label for="newpass" class="control-label col-sm-4">New Password</label>
          <div class="col-sm-8">
            <input type="password" name="newpass" class="form-control" placeholder="New Password" required>
          </div>
        </div>
        <div class="form-group" id="confirm-group">
          <label for="confirmpass" class="control-label col-sm-4">Confirm New Password</label>
          <div class="col-sm-8">
            <input type="password" name="confirmpass" class="form-control" placeholder="Confirm New Password" required>
            <div class="help-block"></div>
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
<script>
  function validatePass() {
    var password = $("input[name='newpass']").val();
    var confirmPassword = $("input[name='confirmpass']").val();

    if (password != confirmPassword ) {
      $("#confirm-group").addClass("has-error");
      $("input[name='confirmpass']").next(".help-block").text("Password does not match!");
      return false;
    } else {
      return true;
    }
  }
  $( document ).ready(function() {
    $("#enabled").on('change', function () {
      if ($(this).is(':checked')) {
        $('input[name="enabled"]').attr('value', 'true');
      } else {
        $('input[name="enabled"]').attr('value', 'false');
      }
    });
  });
</script>
</body>
</html>
