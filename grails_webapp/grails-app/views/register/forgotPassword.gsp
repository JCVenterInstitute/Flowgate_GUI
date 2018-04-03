<html>
<head>
  <meta name="layout" content="${layoutRegister}"/>
  <s2ui:title messageCode='spring.security.ui.forgotPassword.title'/>
  <style>
    .ui-dialog-titlebar {
      display: none;
    }
  </style>
</head>

<body>
<div class="container">
  <g:if test='${emailSent}'>
    <g:message code='spring.security.ui.forgotPassword.sent'/>
  </g:if>
  <g:else>
    <s2ui:formContainer type='forgotPassword' focus='username'>
      <s2ui:form beanName='forgotPasswordCommand'>
        <div class="row justify-content-center">
          <div class="col-12 col-lg-8">
            <h2 class="text-center my-4">Forgot Password?</h2>
            <h3 class="text-center my-4 lead"><g:message code='spring.security.ui.forgotPassword.description'/></h3>
          </div>
        </div>
        <div class="row justify-content-center">
          <div class="col-lg-8">
            <form>
              <div class="form-row justify-content-center">
                <div class="form-group col-md-6">
                  <s2ui:textFieldRow name='username' class="form-control" placeholder="Username" required="" />
                </div>
                <div class="form-group col-md-3">
                  <label></label>
                  <button type="submit" class="btn btn-primary form-control"><g:message code='spring.security.ui.forgotPassword.submit'/></button>
                </div>
              </div>
            </form>
          </div>
        </div>
      </s2ui:form>
    </s2ui:formContainer>
  </g:else>
</div>
</body>
</html>

