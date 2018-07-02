<!doctype html>
<html lang="en" class="no-js">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:stylesheet src="application.css"/>
  %{--<asset:stylesheet href="ofg.css" />--}%
  <g:layoutHead/>
  <sec:ifLoggedIn><script>window.location.href="${createLink(uri: '/project/list')}"</script></sec:ifLoggedIn>
</head>

<body class="text-center">
%{--<g:render template="/layouts/ofgNavBar" />--}%
<g:layoutBody/>
<g:if test="${flash.message}">
  <div class="row justify-content-center ">
    <div class="alert alert-info text-center" role="alert">${flash.message}</div>
  </div>
</g:if>
%{--<div class="footer" role="contentinfo"></div>--}%
<div id="screen-locker" style="display: none;"></div>
<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>

<asset:javascript src="application.js"/>
</body>
</html>
