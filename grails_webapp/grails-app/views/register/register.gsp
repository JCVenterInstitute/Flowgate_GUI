<html>
<head>
    <meta name="layout" content="${layoutRegister}"/>
    <s2ui:title messageCode='spring.security.ui.register.title'/>
</head>
<body>
%{--<br/>--}%
<s2ui:formContainer type='register' focus='username' width='800px' height='320' >
    <s2ui:form beanName='registerCommand' >
        <g:if test='${emailSent}'>
            <br/>
            <g:message code='spring.security.ui.register.sent'/>
        </g:if>
        <g:else>
            <br/>
            <table style="border-top: none">
                <tbody>
                <s2ui:textFieldRow name='username' size='40' labelCodeDefault='Username'/>
                <s2ui:textFieldRow name='email' size='40' labelCodeDefault='E-mail'/>
                <s2ui:passwordFieldRow name='password' size='40' labelCodeDefault='Password'/>
                %{--[Password must have at least one letter, number and special character: !@#$%^&]--}%
                <s2ui:passwordFieldRow name='password2' size='40' labelCodeDefault='Password (again)'/>
                </tbody>
            </table>
            %{--<s2ui:submitButton class='btn btn-default' style="text-decoration: none" elementId='submit' messageCode='spring.security.ui.register.submit'/>--}%
            <s2ui:submitButton class='btn btn-success' style="text-decoration: none" elementId='submit' messageCode='spring.security.ui.register.submit'/>
        </g:else>
    </s2ui:form>
</s2ui:formContainer>
</body>
</html>