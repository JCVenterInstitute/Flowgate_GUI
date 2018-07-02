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
    <h4 class="text-center col-md-8 my-4">FlowGate is being alpha-tested. Before its formal release, the account registration is invitation-only. Plesae send us your comments. We sincerely appreciate your support.</ s>
  </div>
  <div class="row justify-content-center">
    <div class="col-lg-8">
      <g:if test='${flash.error}'>
        <div class="row justify-content-center ">
          <div class="alert alert-danger text-center" role="alert">${flash.error}</div>
        </div>
      </g:if>
      <g:if test='${emailSent}'>
        <div class="row justify-content-center ">
          %{--<div class="alert alert-info text-center" role="alert"><g:message code='spring.security.ui.register.sent'/></div>--}%
          <div class="alert alert-info text-center" role="alert">We received your request! Thank you.</div>
        </div>
      </g:if>
      <g:else>
        <s2ui:formContainer type='register' focus='username'>
          <s2ui:form beanName='registerCommand'>
            <div class="form-row">
              <div class="form-group col-md-6">
                <s2ui:textFieldRow name='username' class="form-control" labelCodeDefault='Your Name' placeholder="Your Name"/>
              </div>
              <div class="form-group col-md-6">
                <s2ui:textFieldRow name='email' class="form-control" labelCodeDefault='Your Email' placeholder="Your Email"/>
              </div>
            </div>
            <div class="form-row">
              %{--<div class="form-group col-md-6">
                <s2ui:textFieldRow name='affiliation' class="form-control" labelCodeDefault='Affiliation' placeholder="Affiliation"/>
              </div>--}%
              <div class="form-group col-md-12">
                <label for="reason">Comments for us</label>
                <g:textArea name='reason' class="form-control" placeholder="Comments for us" rows="5"/>
              </div>
            </div>
            %{--<div class="form-row">
              <div class="form-group col-md-6">
                <s2ui:passwordFieldRow name='password' class="form-control" labelCodeDefault='Password' placeholder="Password"/>
              </div>
              <div class="form-group col-md-6">
                <s2ui:passwordFieldRow name='password2' class="form-control" labelCodeDefault='Confirm Password' placeholder="Confirm Password"/>
              </div>
            </div>--}%
            <div class="form-row">
              <div class="form-group col-md-6">
                <button type="submit" class="btn btn-primary">Submit your comments</button>
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