<!doctype html>
<html class="no-js" lang="">
<head>
  <meta charset="utf-8">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <meta http-equiv="x-ua-compatible" content="ie=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <asset:link rel="icon" href="f.ico" type="image/x-ico" />

  <asset:stylesheet src="ssui.css.less"/>
  <asset:stylesheet src="application.css"/>
  <g:layoutHead/>
</head>

<body>
<g:render template="/layouts/navBar"/>
<g:layoutBody/>

<!-- Footer -->
<footer class="py-3 bg-dark" style="position: fixed;bottom: 0;width: 100%;">
  <div class="container">
    <p class="m-0 text-center text-white">FlowGate &copy; 2018 | <g:link uri="/about"><g:message code="about.link.label" default="About"/></g:link> | <g:link uri="mailto:${grailsApplication.config.adminEmailAddr}" >Contact Us</g:link></p>
  </div>
</footer>

<div id="screen-locker" style="display: none;"></div>
<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>

<asset:javascript src='spring-security-ui-register.js'/>
<asset:javascript src="application.js"/>

</body>
</html>