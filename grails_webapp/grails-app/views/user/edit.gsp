<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.edit.label" args="[entityName]"/></h2>

<div class="row">
  <g:form resource="${user}" method="PUT" class="col s6">
    <g:hasErrors bean="${user}">
      <g:eachError var="err" bean="${user}">
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
      <div class="input-field col s8">
        <input type="text" name="username" value="${user.username}" required>
        <label for="username">Username</label>
      </div>

      <div class="input-field col s8">
        <input type="email" name="email" value="${user.email}" required>
        <label for="email">Email</label>
      </div>

      <div class="input-field col s8">
        <input type="text" name="affiliation" value="${user.affiliation}" required>
        <label for="affiliation">Affiliation</label>
      </div>

      <g:if test="${!user.authorities.authority.contains("ROLE_Admin") && !user.authorities.authority.contains("ROLE_Administrator")}">
        <div class="col s8">
          <label>
            <input type="checkbox" name="_enabled" class="filled-in" <g:if test="${user.enabled}">checked</g:if>/>
            <span>Active</span>
          </label>
        </div>
      </g:if>

      <div class="input-field col s8">
        <button type="submit" class="btn waves-effect waves-light">Update User</button>
        <g:link controller="user" action="list" class="btn-flat">Return to User List</g:link>
      </div>
    </div>
  </g:form>

  <g:form resource="${user}" method="PUT" action="updatePassword" class="col s6" onsubmit="return validatePass()">
    <g:if test="${flash.passError}">
      <div class="alert alert-danger">${flash.passError}</div>
    </g:if>

    <div class="row">
      <div class="input-field col s8">
        <input type="password" name="oldpass" required>
        <label for="oldpass">Old Password</label>
      </div>

      <div class="input-field col s8">
        <input type="password" name="newpass" required>
        <label for="newpass">New Password</label>
      </div>

      <div class="input-field col s8">
        <input type="password" name="confirmpass" required>
        <label for="confirmpass">Confirm New Password</label>

        <div class="help-block"></div>
      </div>

      <div class="input-field col s8">
        <button type="submit" class="btn waves-effect waves-lighty">Update Password</button>
      </div>
    </div>
  </g:form>
</div>

<script>
  function validatePass() {
    var password = $("input[name='newpass']").val();
    var confirmPassword = $("input[name='confirmpass']").val();

    if (password != confirmPassword) {
      $("#confirm-group").addClass("has-error");
      $("input[name='confirmpass']").next(".help-block").text("Password does not match!");
      return false;
    } else {
      return true;
    }
  }

  $(document).ready(function () {
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
