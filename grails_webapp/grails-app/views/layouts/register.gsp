<!doctype html>
<html class="no-js" lang="">
    <head>
        <meta charset="utf-8">
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
        <meta http-equiv="x-ua-compatible" content="ie=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1">

        <asset:stylesheet src="ssui.css.less"/>
        <s2ui:stylesheet src='spring-security-ui-register.css'/>
        <asset:stylesheet src="application.css"/>
        <g:layoutHead/>
    </head>
    <body>
        <div class="content-wrapper">
            <g:render template="/layouts/navBar" />
            <br/>
            <g:layoutBody/>
        </div>%{--wrapper--}%

        <div class="footer" role="contentinfo"></div>

        <div id="spinner" class="spinner" style="display:none;">
            <g:message code="spinner.alt" default="Loading&hellip;"/>
        </div>

        <asset:javascript src='spring-security-ui-register.js'/>
        <asset:javascript src="application.js"/>
        <s2ui:showFlash/>
        <s2ui:deferredScripts/>
    </body>
</html>