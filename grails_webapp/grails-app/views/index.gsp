<!DOCTYPE html>
<g:set var='securityConfig' value='${applicationContext.springSecurityService.securityConfig}'/>
<html lang="en">
    <head>
        <meta name="layout" content="ofg"/>
    </head>
    <body>
    <g:render template="/shared/nav" />
    <div class="container" style="width:100%;height:98.4%;background-color:#f8f8f8;">
    <br/>
        <div class="row" style="height:900px;">
            <div id="imagebox" class="col-sm-8">
                <!--<div style="width:100%;height:100%;">--}%
                    <img src="../../images/login_bg1.png" height="100%"/>
                </div>-->
            </div>
            <div id="loginbox" class="col-sm-4">
                <div class="row">
                    <div class="col-sm-12">
                        <div class="alert alert-warning" style="margin-top: 5px;">
                            The system is currently under testing and will be released in ...
                        </div>
                        <div id="loginAlert" style="margin-top: 5px;"></div>
                        <div style="margin-bottom: 25px;">
                            %{--<h3>Sign In</h3>--}%
                            <h3><g:message code='spring.security.ui.login.signin'/></h3>
                        </div>
                        <div style="display:none" id="login-alert" class="alert alert-danger col-sm-12"></div>
                        <s2ui:form type='login' focus='username'>

                            <div style="margin-bottom: 25px" class="input-group">
                                <span class="input-group-addon"><i class="glyphicon glyphicon-user"></i></span>
                                <input type="text" name="${securityConfig.apf.usernameParameter}" id="username" placeholder="email" class='input-sm' size="40"/>
                            </div>

                            <div style="margin-bottom: 25px" class="input-group">
                                <span class="input-group-addon"><i class="glyphicon glyphicon-lock"></i></span>
                                <input type="password" name="${securityConfig.apf.passwordParameter}" id="password" class="input-sm" size="40"/>

                            </div>

                            <div style="margin-top:10px" class="form-group">
                                <div class="col-sm-12 controls">
                                    %{--<input type="hidden" name="j" value="u_l" />--}%
                                    <input id="loginButton_submit" class=' btn-primary btn-lg' value="${g.message( code:'spring.security.ui.login.login' )}" type="submit" >
                                </div>
                            </div>
                            <br/>
                            <div class="form-group" style="margin-top:35px;margin-bottom:40px;">
                                <div class="col-md-12 control">
                                    <div style="border-top: 1px solid#888; padding-top:5px; font-size:1em;" >
                                        <g:link controller="register" action="forgotPassword"><g:message code="spring.security.ui.login.forgotPw" default="Forgot Password?" /></g:link>
                                    </div>
                                </div>
                                <div class="col-md-12 control">
                                    <div style="font-size:1em;" >
                                        Don't have an account?
                                        <s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/>
                                    </div>
                                </div>
                            </div>

                        </s2ui:form>
                    </div>
                </div>
            </div>
        </div>
    </div>
    </div>

    %{--<script src="../../js/jquery.min.js"></script>--}%
    %{--<script src="../../js/shared.js"></script>--}%
    %{--<script src="../../js/bootstrap.min.js"></script>--}%
    %{--<script>--}%
        %{--$(function(){--}%
            %{--$("#nav").load("../common/nav.php");--}%
        %{--});--}%
    %{-- </script> --}%
    </body>
</html>