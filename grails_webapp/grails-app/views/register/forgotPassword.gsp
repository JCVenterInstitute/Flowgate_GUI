<html>
<head>
  <meta name="layout" content="main"/>
  <s2ui:title messageCode='spring.security.ui.forgotPassword.title'/>
  <style>
  .ui-dialog-titlebar {
    display: none;
  }
  </style>
</head>

<body>
<h2>Forgot Password</h2>

<h5><g:message code='spring.security.ui.forgotPassword.description'/></h5>
<g:hasErrors bean="${forgotPasswordCommand}">
  <g:eachError var="err" bean="${forgotPasswordCommand}">
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
<g:if test="${emailSent}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var messageToastHTML = '<span>We have sent the password recovery email.</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: messageToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>
<g:else>
  <div class="row">
    <s2ui:formContainer type='forgotPassword' focus='username'>
      <s2ui:form beanName='forgotPasswordCommand'>
        <div class="input-field col s6">
          <input type="text" name="username" required>
          <label for="username">Username</label>
        </div>

        <div class="input-field col s12">
          <button type="submit" class="btn waves-effect waves-light"><g:message code='spring.security.ui.forgotPassword.submit'/></button>
        </div>
      </s2ui:form>
    </s2ui:formContainer>
  </div>
</g:else>
</body>
</html>

