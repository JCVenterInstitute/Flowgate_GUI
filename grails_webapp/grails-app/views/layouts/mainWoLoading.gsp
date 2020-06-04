<!doctype html>
<html lang="en" class="no-js">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="favicon.ico" type="image/x-ico" />
  <asset:stylesheet src="application.css"/>
  <asset:javascript src="jquery-2.2.0.min.js"/>
  <g:layoutHead/>
</head>

<body>
<g:render template="/layouts/navBar"/>
<g:ifPageProperty name="page.topBtnBar">
  <div class="container" style="position: static">
    <g:pageProperty name="page.topBtnBar" />
  </div>
</g:ifPageProperty>
<g:if test="${flash.message}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var successToastHTML = '<span>${flash.message}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: successToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>
<g:layoutBody/>

<!-- Footer -->
<footer class="py-3 bg-dark" style="position: fixed;bottom: 0;width: 100%;">
  <div class="container">
    <p class="m-0 text-center text-white">FlowGate &copy; 2020 | <g:link uri="/about" ><g:message code="about.link.label" default="About" /></g:link> | <g:link uri="mailto:${grailsApplication.config.adminEmailAddr}" >Contact Us</g:link></p>
  </div>
</footer>

<div id="screen-locker" style="display: none;"></div>

<asset:javascript src="application.js"/>
</body>
</html>
