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

    <f:with bean="user">
      <div class="col s8">
        <div class="row">
          <f:field property="username" label="Username" required="true" value="${user.username}"/>
          <f:field property="email" email="true" required="true" value="${user.email}"/>
          <f:field property="affiliation" required="true" value="${user.affiliation}"/>

          <g:if test="${!user.authorities.authority.contains("ROLE_Admin") && !user.authorities.authority.contains("ROLE_Administrator")}">
            <div class="col s8">
              <label>
                <input type="checkbox" name="_enabled" class="filled-in" <g:if test="${user.enabled}">checked</g:if>/>
                <span>Enabled</span>
              </label>
            </div>
          </g:if>
        </div>
      </div>
    </f:with>

    <div class="row">
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
        <input type="password" name="oldpass" id="oldpass" required>
        <label for="oldpass">Old Password*</label>
      </div>

      <div class="input-field col s8">
        <input type="password" name="newpass" id="newpass" required>
        <label for="newpass">New Password*</label>
      </div>

      <div class="input-field col s8">
        <input type="password" name="confirmpass" id="confirmpass" required>
        <label for="confirmpass">Confirm New Password*</label>

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
