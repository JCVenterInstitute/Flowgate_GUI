<g:set var='securityConfig' value='${applicationContext.springSecurityService.securityConfig}'/>
<html>
<head>
  %{--<meta name="layout" content="${layoutRegister}"/>--}%
  <meta name="layout" content="main"/>
  <s2ui:title messageCode='spring.security.ui.login.title'/>
  <style>
  .form-signin {
    width: 100%;
    max-width: 330px;
    padding: 15px;
    margin: 0 auto;
  }

  .form-signin .form-control {
    position: relative;
    box-sizing: border-box;
    height: auto;
    padding: 10px;
    font-size: 16px;
  }

  .form-signin input[type="text"] {
    margin-bottom: -1px;
    border-bottom-right-radius: 0;
    border-bottom-left-radius: 0;
  }

  .form-signin input[type="password"] {
    margin-bottom: 10px;
    border-top-left-radius: 0;
    border-top-right-radius: 0;
  }</style>
</head>

<body>

<div class="container">
  <div class="row justify-content-center ">
    <h2 class="text-center my-4"><g:message code='spring.security.ui.login.signin'/></h2>
  </div>

  <div class="row justify-content-center">
    <div class="col-lg-8 text-center">
      <s2ui:form type='login' focus='username' class="form-signin">
        <label for="username" class="sr-only">Username</label>
        <input type="text" name="${securityConfig.apf.usernameParameter}" id="username" placeholder="Username" class="form-control" required="" autofocus=""/>
        <label for="password" class="sr-only">Password</label>
        <input type="password" name="${securityConfig.apf.passwordParameter}" id="password" placeholder="Password" class="form-control" required=""/>
        %{--
        <p id="remember_me_holder">
          <input type="checkbox" class="chk" name="${rememberMeParameter ?: 'remember-me'}" id="remember_me" <g:if test='${hasCookie}'>checked="checked"</g:if>/>
          <label for="remember_me"><g:message code='springSecurity.login.remember.me.label'/></label>
        </p>
        --}%
        <div class="mb-3"></div>
          <button class="btn btn-lg btn-primary btn-block" type="submit">${g.message(code: 'spring.security.ui.login.login')}</button>
        <div>
          <s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/>&nbsp;or&nbsp;
          <g:link controller="register" action="forgotPassword">
            <g:message code="spring.security.ui.login.forgotPw" default="Reset password"/>
          </g:link>
        </div>
      </s2ui:form>
    </div>
  </div>
</div>
<script>
  $(document).ready(function() {
    $('.nav-item').removeClass('active');
    $('#navbar-login').addClass('active');
  });
</script>
</body>
</html>