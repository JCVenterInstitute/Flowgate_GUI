<html>
<head>
    <meta name="layout" content="${layoutRegister}"/>
    <s2ui:title messageCode='spring.security.ui.resetPassword.title'/>
</head>
<body>
<s2ui:formContainer type='resetPassword' focus='password' width='475px' height="300">
    <s2ui:form beanName='resetPasswordCommand'>
        <g:hiddenField name='t' value='${token}'/>
        <div class="sign_in">
            <br/>
            <h3><g:message code='spring.security.ui.resetPassword.description'/></h3>
            <table style="border-top: none">
                <s2ui:passwordFieldRow name='password' labelCodeDefault='Password'/>
                <s2ui:passwordFieldRow name='password2' labelCodeDefault='Password (again)'/>
            </table>
            <s2ui:submitButton class="btn btn-success" style="text-decoration: none" elementId='submit' messageCode='spring.security.ui.resetPassword.submit'/>
        </div>
    </s2ui:form>
</s2ui:formContainer>
</body>
</html>