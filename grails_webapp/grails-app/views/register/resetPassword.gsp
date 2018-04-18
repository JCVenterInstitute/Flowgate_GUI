<html>
<head>
  <meta name="layout" content="${layoutRegister}"/>
  <s2ui:title messageCode='spring.security.ui.resetPassword.title'/>
  <style>
  .ui-dialog-titlebar {
    display: none;
  }
  </style>
</head>

<body>
<div class="container">
  <div class="row justify-content-center">
    <h2 class="text-center my-4">Reset Password</h2>
  </div>
  <div class="row justify-content-center">
    <div class="col-lg-8">
      <s2ui:formContainer type='resetPassword' focus='password'>
        <s2ui:form beanName='resetPasswordCommand'>
          <g:hiddenField name='t' value='${token}'/>

          <div class="form-row">
            <div class="form-group col-md-6">
              <s2ui:passwordFieldRow name='password' class="form-control" labelCodeDefault='Password'/>
            </div>

            <div class="form-group col-md-6">
              <s2ui:passwordFieldRow name='password2' class="form-control" labelCodeDefault='Password (again)'/>
            </div>
          </div>

          <div class="form-row">
            <div class="form-group col-md-6">
              <button type="submit" class="btn btn-primary"><g:message code='spring.security.ui.resetPassword.submit'/></button>
            </div>
          </div>
        </s2ui:form>
      </s2ui:formContainer>
    </div>
  </div>
</div>
</body>
</html>