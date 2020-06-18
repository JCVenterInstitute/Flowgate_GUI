<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'User')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.create.label" args="[entityName]"/></h2>

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
  <g:form resource="${user}" action="save" class="col s6" onsubmit="return validatePass()">
    <div class="row">
      <div class="input-field col s12">
        <input type="text" name="username" value="${user.username}" required>
        <label for="username">Username</label>
      </div>

      <div class="input-field col s12">
        <input type="email" name="email" value="${user.email}" required>
        <label for="email">Email</label>
      </div>

      <div class="input-field col s12">
        <input type="text" name="affiliation" value="${user.affiliation}" required>
        <label for="affiliation">Affiliation</label>
      </div>

      <div class="input-field col s12">
        <input type="text" name="reason" value="${user.reason}" required>
        <label for="reason">Reason</label>
      </div>

      <div class="col s12">
        <label>
          <input type="hidden" name="_enabled" value="${user.enabled}">
          <input type="checkbox" class="filled-in" <g:if test="${user.enabled}">checked</g:if>/>
          <span>Enabled</span>
        </label>
      </div>

      <div class="input-field col s12">
        <input type="password" name="password" required>
        <label for="password">Password</label>
      </div>

      <div class="input-field col s12">
        <input type="password" name="confirmpass" required>
        <label for="confirmpass">Confirm Password</label>

        <div class="help-block"></div>
      </div>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Create User</button>
        <g:link action="list" class="btn-flat">Return to User List</g:link>
      </div>
    </div>
  </g:form>
</div>

<script>
  function validatePass() {
    var password = $("input[name='password']").val();
    var confirmPassword = $("input[name='confirmpass']").val();

    if (password != confirmPassword) {
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