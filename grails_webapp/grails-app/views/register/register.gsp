<html>
<head>
  <meta name="layout" content="main"/>
  <s2ui:title messageCode='spring.security.ui.register.title' entityNameDefault="FlowGate - Registration"/>
  <style>
    .ui-dialog-titlebar {
      display: none;
    }
  </style>
</head>

<body>
<h2>Registration</h2>

<h5>FlowGate is being alpha-tested. Before its formal release, the account registration is invitation-only. Please send us your comments. We sincerely appreciate your support.</h5>

<g:if test="${emailSent}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var messageToastHTML = '<span>We received your request! Thank you.</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: messageToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>
<g:else>
  <div class="row">
    <s2ui:formContainer type='register' focus='username'>
      <s2ui:form beanName='registerCommand'>
        <div class="input-field col s12">
          <input type="text" name="username" id="username" required>
          <label for="username">Full name</label>
        </div>

        <div class="input-field col s12">
          <input type="email" name="email" id="email" required>
          <label for="email">Email</label>
        </div>

        <div class="input-field col s12">
          <textarea name="reason" id="reason" class="materialize-textarea"></textarea>
          <label for="reason">Comments for us</label>
        </div>

        <div class="input-field col s12">
          <button type="submit" class="btn waves-effect waves-light">Submit Request</button>
          <a href="${createLink(uri: '/')}" class="btn-flat">Return to Login</a>
        </div>
      </s2ui:form>
    </s2ui:formContainer>
  </div>
</g:else>
</body>
</html>
