<!doctype html>
<html lang="en" class="no-js">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico"/>
  <asset:stylesheet src="application.css"/>
  <g:layoutHead/>
</head>

<body>
<div class="content-wrapper">
  <g:render template="/layouts/navBar"/>
  <g:layoutBody/>
</div>%{--wrapper--}%
<g:if test="${flash.message}">
  <div class="row justify-content-center ">
    <div class="alert alert-info text-center" role="alert">${flash.message}</div>
  </div>
</g:if>

<!-- Footer -->
<footer class="py-3 bg-dark" style="position: fixed;bottom: 0;width: 100%;">
  <div class="container">
    <p class="m-0 text-center text-white">FlowGate &copy; 2019 | <g:link uri="/about"><g:message code="about.link.label" default="About"/></g:link> | <g:link uri="mailto:${grailsApplication.config.adminEmailAddr}" >Contact Us</g:link></p>
  </div>
</footer>

<div id="screen-locker" style="display: none;"></div>
<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>

<asset:javascript src="application.js"/>
</body>
</html>
