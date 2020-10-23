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

    <f:with bean="user">
      <div class="row">
        <f:field property="username" label="Username" required="true" value="${user.username}"/>
        <f:field property="email" email="true" required="true" value="${user.email}"/>
        <f:field property="affiliation" required="true" value="${user.affiliation}"/>
        <f:field property="reason" required="true" value="${user.reason}"/>

        <div class="col s12">
          <label>
            <input type="checkbox" name="_enabled" class="filled-in" <g:if test="${user.enabled}">checked</g:if>/>
            <span>Enabled</span>
          </label>
        </div>

        <f:field property="password" password="true" label="Password" required="true" value="${user.reason}"/>

        <div class="input-field col s12">
          <input type="password" name="confirmpass" id="confirmpass" required>
          <label for="confirmpass">Confirm Password*</label>

          <div class="help-block"></div>
        </div>
      </div>
    </f:with>

    <div class="row">
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
