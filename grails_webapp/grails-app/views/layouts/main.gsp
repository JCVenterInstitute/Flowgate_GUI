<!doctype html>
<html lang="en" class="no-js">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="favicon.ico" type="image/x-ico"/>
  <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
  <asset:stylesheet src="application.css"/>
  <asset:javascript src="jquery-2.2.0.min.js"/>
  <g:layoutHead/>

  <style>
  .scrollButton {
    position: fixed;
    right: 23px;
    background: #3f51b5;
    border-radius: 30%;
    display: none;
  }

  .scrollButton:hover, .scrollButton:active, .scrollButton:focus {
    background-color: rgb(51 66 146);
  }
  </style>
</head>

<body>
<g:render template="/layouts/navBar"/>

<a href="#" class="scrollButton btn-floating" id="scrollToTop" style="top:75px;"><i class="material-icons">arrow_drop_up</i></a>
<a href="#" class="scrollButton btn-floating" id="scrollToBottom" style="top:125px;"><i class="material-icons">arrow_drop_down</i></a>

<g:if test="${flash.success}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var successToastHTML = '<span>${flash.success}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: successToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>
<g:if test="${flash.message}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var messageToastHTML = '<span>${flash.message}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: messageToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>
<g:if test="${flash.error}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      M.toast({
        html: '<span>${flash.error}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
        displayLength: Infinity,
        classes: 'red'
      });
    });
  </script>
</g:if>
<g:ifPageProperty name="page.topBtnBar">
  <div class="container" style="position: static">
    <g:pageProperty name="page.topBtnBar"/>
  </div>
</g:ifPageProperty>
<div class="container">
  <div class="row">
    <div class="col s12">
      <g:layoutBody/>
    </div>
  </div>
</div>

<!-- Footer -->
<div class="preload-background modal-overlay"
     style="display: none;align-items: center;justify-content: center;position: fixed;z-index: 100;top: 0;opacity: 0.5;">
  <div class="preloader-wrapper big active">
    <div class="spinner-layer spinner-blue-only">
      <div class="circle-clipper left">
        <div class="circle"></div>
      </div>

      <div class="gap-patch">
        <div class="circle"></div>
      </div>

      <div class="circle-clipper right">
        <div class="circle"></div>
      </div>
    </div>
  </div>
</div>

<asset:javascript src="application.js"/>
</body>
</html>
