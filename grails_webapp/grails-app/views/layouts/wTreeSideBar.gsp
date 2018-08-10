<!doctype html>
<html lang="en" class="no-js" style="height: 100%">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico"/>
  <asset:stylesheet src="application.css"/>
  <asset:javascript src="jquery-2.2.0.min.js"/>
  <g:layoutHead/>
  <style>
    /* Hide for mobile, show later */
    .sidebar {
      display: none;
    }

    .main {
      padding: 20px;
    }

    @media (min-width: 768px) {
      .sidebar {
        position: fixed;
        top: 52px;
        bottom: 0;
        left: 0;
        z-index: 1000;
        display: block;
        overflow-x: hidden;
        overflow-y: auto; /* Scrollable contents if viewport is shorter than content. */
        background-color: #f5f5f5;
        border-right: 1px solid #eee;
      }

      .main {
        padding-right: 40px;
        padding-left: 40px;
      }
    }

    .main .page-header {
      margin-top: 0;
    }

    #projTree > ul {
      padding-left: 0px;
      list-style-type: none;
      list-style: none;
    }

    #projTree > ul > ul {
      padding-left: 20px;
      list-style-type: none;
      list-style: none;
    }
  </style>
  <script>
    $(function() {
      checkScroll();
      $(window).scroll(function() {
        checkScroll();
      });
    });

    function checkScroll() {
      var scroll = $(window).scrollTop();
      if(scroll > 52) {
        $(".sidebar").css("top", 0);
        $(".sidebar #projTree").css("margin-top", "60px");
        $("#tree-brand").fadeIn( "100");
      } else {
        $(".sidebar").css("top", 52 - scroll);
        $(".sidebar #projTree").css("margin-top", "0px");
        $("#tree-brand").hide();
      }
    }
  </script>
</head>

<body>
<div class="content-wrapper">
  <g:render template="/layouts/navBar"/>
  <div class="container-fluid">
    <div class="row">
      <div class="col-sm-3 col-md-2 sidebar">
        <div id="tree-brand" style="display: none;background-color: #343a40 !important;position: absolute;left:0;right:0;">
          <a class="navbar-brand" href="${createLink(uri: '/')}" style="padding-left: 20%;">
            <img src="${assetPath(src: 'logo-dark.png')}" style="height: 40px;" class="img-fluid">
          </a>
        </div>
        <g:pageProperty name="page.treeView"/>
      </div>
      <div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main">
        <g:pageProperty name="page.topBtnBar"/>
        <g:pageProperty name="page.pgContent"/>
      </div>
    </div>
  </div>
</div>%{--wrapper--}%
%{--<div class="footer" role="contentinfo"></div>--}%
<div id="screen-locker" style="display: none;"></div>
<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>
<asset:javascript src="application.js"/>
<g:render template="/layouts/wTreeSidebarScripts"/>
<g:pageProperty name="page.pgAssets"/>
</body>
</html>
