<!doctype html>
<html lang="en" class="no-js">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
    <title><g:layoutTitle default="flowgate"/></title>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <asset:stylesheet src="application.css"/>
    <asset:javascript src="uploadr.manifest.js" />
    <asset:javascript src="uploadr.demo.manifest.js" />
    <asset:stylesheet href="uploadr.manifest.css" />
    <asset:stylesheet href="uploadr.demo.manifest.css" />
    %{--<asset:stylesheet src="application.css"/>--}%
    <g:layoutHead/>
</head>
<body>
    <div class="content-wrapper">
        <g:render template="/layouts/navBar" />
        <g:layoutBody/>
    </div>%{--wrapper--}%
    <g:if test="${flash.message}">
        <div id="msg" class="text-center message" role="status">${flash.message}</div>
    </g:if>
    <div class="footer" role="contentinfo"></div>

    <div id="spinner" class="spinner" style="display:none;">
        <g:message code="spinner.alt" default="Loading&hellip;"/>
    </div>

    %{--<asset:javascript src="application.js"/>--}%
</body>
</html>
