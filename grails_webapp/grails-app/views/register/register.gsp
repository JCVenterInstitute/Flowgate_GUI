<html>
<head>
  <meta name="layout" content="${layoutRegister}"/>
  <s2ui:title messageCode='spring.security.ui.register.title' entityNameDefault="FlowGate - Registration"/>
  <style>
    .ui-dialog-titlebar {
      display: none;
    }
  </style>
</head>

<body>
<div class="container">
  <div class="row justify-content-center">
    <h2 class="text-center my-4">Registration</h2>
  </div>
  <div class="row justify-content-center">
    <div class="col-lg-8">
      <g:if test='${emailSent}'>
        <div class="row justify-content-center ">
          <div class="alert alert-info text-center" role="alert"><g:message code='spring.security.ui.register.sent'/></div>
        </div>
      </g:if>
      <g:else>
        <s2ui:formContainer type='register' focus='username'>
          <s2ui:form beanName='registerCommand'>
            <div class="form-row">
              <div class="form-group col-md-6">
                <s2ui:textFieldRow name='username' class="form-control" labelCodeDefault='Username' placeholder="Username"/>
              </div>
              <div class="form-group col-md-6">
                <s2ui:textFieldRow name='email' class="form-control" labelCodeDefault='Email' placeholder="Email"/>
              </div>
            </div>
            <div class="form-row">
              <div class="form-group col-md-6">
                <s2ui:passwordFieldRow name='password' class="form-control" labelCodeDefault='Password' placeholder="Password"/>
              </div>
              <div class="form-group col-md-6">
                <s2ui:passwordFieldRow name='password2' class="form-control" labelCodeDefault='Confirm Password' placeholder="Confirm Password"/>
              </div>
            </div>
            <div class="form-row">
              <div class="form-group col-md-6">
                <button type="submit" class="btn btn-primary">Create Your Account</button>
              </div>
            </div>
          </s2ui:form>
        </s2ui:formContainer>
      </g:else>
    </div>
  </div>
</div>
</body>
</html>