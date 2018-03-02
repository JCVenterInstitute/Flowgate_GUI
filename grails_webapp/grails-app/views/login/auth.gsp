<g:set var='securityConfig' value='${applicationContext.springSecurityService.securityConfig}'/>
<html>
    <head>
        %{--<meta name="layout" content="${layoutRegister}"/>--}%
        <meta name="layout" content="main"/>
        <s2ui:title messageCode='spring.security.ui.login.title'/>
        <asset:stylesheet src='spring-security-ui-auth.css'/>
    </head>
    <body>
    <p/>
    <br/>
    <div class="row">
    <div class="col-sm-offset-3 col-sm-12 text-center">
    <div class="login s2ui_center ui-corner-all" style='text-align:center;'>
        <div class="login-inner">
            <s2ui:form type='login' focus='username'>
                <div class="sign-in">
                    <h2><g:message code='spring.security.ui.login.signin'/></h2>
                    <br/>
                    <div class="row">
                        <div class="col-sm-offset-0 col-sm-12 input-group-sm">
                            <div class="col-sm-2" ><label for="username"><g:message code='spring.security.ui.login.username'/></label></div>
                            <div class="col-sm-10" ><input type="text" name="${securityConfig.apf.usernameParameter}" id="username" class='input-sm' size="40"/></div>
                        </div>
                    </div>
                    <div class="row">
                        <div class="col-sm-offset-0 col-sm-12 input-group-sm">
                            <div class="col-sm-2" ><label for="password"><g:message code='spring.security.ui.login.password'/></label></div>
                            <div class="col-sm-10" ><input type="password" name="${securityConfig.apf.passwordParameter}" id="password" class="input-sm" size="40"/></div>
                        </div>
                    </div>
                    <div class="row">
                        <div class="col-sm-offset-0 col-sm-12">
                        <g:link controller="register" action="forgotPassword">Forgot your password?</g:link>
                        </div>
                    </div>
                    %{--<br/>--}%
                    %{--<div class="row">--}%
                        %{--<div class="col-sm-offset-0 col-sm-12 input-group-sm">--}%
                            %{--
                            <div class="col-sm-1" >
                                <input type="checkbox" class="checkbox" name="${securityConfig.rememberMe.parameter}" id="remember_me" checked="checked"/>
                            </div>
                            <div class="col-sm-4" >
                                <label for='remember_me'><g:message code='spring.security.ui.login.rememberme'/></label>
                            </div>
                            <div class="col-sm-6" >
                                <span class="forgot-link">
                                    <g:link controller='register' action='forgotPassword'><g:message code='spring.security.ui.login.forgotPassword'/></g:link>
                                </span>
                            </div>
                            --}%
                        %{--</div>--}%
                    %{--</div>--}%
                    <br/>
                    <div class="row">
                        <div class="col-sm-offset-0 col-sm-12 input-group-sm">
                            New User? <s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/>
                        </div>
                    </div>
                    <br/>
                    <div class="row">
                        <div class="col-sm-offset-0 col-sm-12 input-group-sm">
                            %{-- original tag <s2ui:submitButton elementId='loginButton' messageCode='spring.security.ui.login.login'/>--}%
                            %{-- modified does not work because of taglib <s2ui:submitButton class='btn btn-success' elementId='loginButton' messageCode='spring.security.ui.login.login'/>--}%
                            <input id="loginButton_submit" class='btn btn-success' value="${g.message( code:'spring.security.ui.login.login' )}" type="submit" >
                        </div>
                    </div>
                </div>
            </s2ui:form>
        </div>
        </div>
        </div>


    </div>
    </body>
</html>