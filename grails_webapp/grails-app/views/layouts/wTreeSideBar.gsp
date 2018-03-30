<!doctype html>
<html lang="en" class="no-js" style="height: 100%">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
  <title><g:layoutTitle default="flowgate"/></title>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico"/>
  <asset:stylesheet src="application.css"/>
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
  </style>
</head>

<body>
<div class="content-wrapper">
  <g:render template="/layouts/navBar"/>
  %{--
  <div class="navbar navbar-default navbar-static-top" role="navigation">
       <div class="navbar-header">
           <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
               <span class="sr-only">Toggle navigation</span>
               <span class="icon-bar"></span>
               <span class="icon-bar"></span>
               <span class="icon-bar"></span>
           </button>
           <a class="navbar-brand" href="/#">
               --}%%{-- <i class=""><asset:image src="logo"/></i> --}%%{--
               <g:if env="production">flowgate</g:if>
           </a>
       </div>
       <div class="navbar-collapse collapse" aria-expanded="false" style="height: 0.8px;">
           <ul class="nav navbar-nav navbar-right pull-right">
               <g:pageProperty name="page.nav" />
               <div class="pull-right"  style="margin-right: 10px">
                   <sec:ifLoggedIn>
                       <li class="">
                           --}%%{--<li class="navbar-link">--}%%{--
                           <div class="nav-item" style="padding-top: 4px;color: white"><i class="glyphicon glyphicon-user"></i>&nbsp;<sec:username /></div>
                       </li>
                       <li class="navbar-link">
                           <g:link controller="logout">Logout</g:link>
                           --}%%{-- for old version only now E: tag does not exist! <g:remoteLink class="logout" controller="logout">${message(code: 'springSecurity.logout.link')}</g:remoteLink> --}%%{--
                       </li>
                   </sec:ifLoggedIn>
                   <sec:ifNotLoggedIn>
                       <li class="navbar-link">
                           <g:link controller='login' action='auth'>Login</g:link>
                       </li>
                   </sec:ifNotLoggedIn>
               </div>
           </ul>
       </div>
   </div>
   --}%
  %{--TODO divider for the whole page--}%
  %{--<div class="row vDividerBetween" style="padding: 0; margin: 0">--}%
  %{--<div class="row" style="height: 88%; min-height: 88%">--}%
  <div class="container-fluid">
    <div class="row">
      <div class="col-sm-3 col-md-2 sidebar">
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
<div id="spinner" class="spinner" style="display:none;">
  <g:message code="spinner.alt" default="Loading&hellip;"/>
</div>
<asset:javascript src="application.js"/>
<g:render template="/layouts/wTreeSidebarScripts"/>
</body>
</html>
