<!doctype html>
<html lang="en" class="no-js" style="height: 100%">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
        <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
        <title><g:layoutTitle default="flowgate"/></title>
        <meta name="viewport" content="width=device-width, initial-scale=1"/>
        <asset:stylesheet src="application.css"/>
        <asset:stylesheet src="treeView.css.less"/>
        <g:layoutHead/>
    </head>
    <body style="height: 100%;min-height: 100%;margin: 0;padding: 0;">
        <div class="content-wrapper" style="height:100%">
            <g:render template="/layouts/navBar" />
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
            <div class="row sidebarblock" style="padding: 0; margin: 0; max-width: none">

                <div class="col-sm-2" style="position: static">
                    <g:pageProperty name="page.treeView" />
                </div>
                %{--
                <div class="" style="width: 1px; color: #000; border-left: solid 1px; border-right: solid 1px">
                     style="border-left: #000 solid 1px"
                </div>
                --}%
                <div class="col-sm-10" style="position: static">
                    <g:pageProperty name="page.topBtnBar" />
                    <br/>
                    <g:pageProperty name="page.pgContent" />
                </div>
            </div>
        </div>%{--wrapper--}%
        %{--<div class="footer" role="contentinfo"></div>--}%
        <div id="spinner" class="spinner" style="display:none;">
            <g:message code="spinner.alt" default="Loading&hellip;"/>
        </div>
        <asset:javascript src="application.js"/>
        <g:render template="/layouts/wTreeSidebarScripts" />
    </body>
</html>
