<html>
<head>
    <meta name="layout" content="${layoutRegister}"/>
    <s2ui:title messageCode='spring.security.ui.forgotPassword.title'/>
</head>
<body>
<s2ui:formContainer type='forgotPassword' focus='username' width='50%' height="300">
    <s2ui:form beanName='forgotPasswordCommand'>
        <g:if test='${emailSent}'>
            <br/>
            <g:message code='spring.security.ui.forgotPassword.sent'/>
        </g:if>
        <g:else>
            <br/>
            <h3><g:message code='spring.security.ui.forgotPassword.description'/></h3>
            <br />
            %{--<table style="border-top: none">--}%
                %{--<s2ui:textFieldRow name='username' size='40' labelCodeDefault='Username'/>--}%
            %{--</table>--}%
            <div class="row">
                <div class="col-xs-offset-2 col-xs-3 right">
                    <label for="username" >Username</label>
                </div>
                <div class="col-xs-6 left">
                    <input class="form-control" id="username" name="username" value="" size="" type="text" />
                </div>
            </div>
            <br/>
            <br/>
            <s2ui:submitButton class="btn btn-success" style="text-decoration: none" elementId='submit' messageCode='spring.security.ui.forgotPassword.submit'/>
        </g:else>
    </s2ui:form>
</s2ui:formContainer>
</body>
</html>

