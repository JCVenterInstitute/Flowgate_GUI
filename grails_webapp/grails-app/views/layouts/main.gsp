<!doctype html>
<html lang="en" class="no-js">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico" />
  %{--<asset:javascript src="uploadr.manifest.js"/>--}%
  %{--<asset:javascript src="uploadr.demo.manifest.js"/>--}%
  %{--<asset:stylesheet href="uploadr.manifest.css"/>--}%
  %{--<asset:stylesheet href="uploadr.demo.manifest.css"/>--}%
  <asset:stylesheet src="application.css"/>
  <g:layoutHead/>
</head>

<body>
<g:render template="/layouts/navBar"/>
<div class="col-sm-10" style="position: static">
  <g:pageProperty name="page.topBtnBar" />
</div>
<g:if test="${flash.message}">
  <div class="row justify-content-center ">
    <div class="alert alert-info text-center" role="alert">${flash.message}</div>
  </div>
</g:if>
<g:layoutBody/>

<!-- Footer -->
<footer class="py-3 bg-dark" style="position: absolute;bottom: 0;width: 100%;">
  <div class="container">
    <p class="m-0 text-center text-white">FlowGate &copy; 2018 | <g:link uri="/about" ><g:message code="about.link.label" default="About" /></g:link></p>
  </div>
</footer>

<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>

<asset:javascript src="application.js"/>
</body>
</html>
