<!doctype html>
<html lang="en" class="no-js">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
    <title><g:layoutTitle default="flowgate"/></title>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <asset:stylesheet src="application.css"/>
    <g:layoutHead/>
</head>
<body>
    <div class="content-wrapper">
        <div class="navbar navbar-default navbar-static-top" role="navigation">
            <div class="container">
                <div class="navbar-header">
                    <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
                        <span class="sr-only">Toggle navigation</span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                    </button>
                    <a class="navbar-brand" href="/#">
                        <i class="fa grails-icon">
                            %{--<asset:image src="grails-cupsonly-logo-white.svg"/>--}%
                        </i><g:if env="production">flowgate</g:if>
                    </a>
                </div>
                <div class="navbar-collapse collapse" aria-expanded="false" style="height: 0.8px;">
                    <ul class="nav navbar-nav navbar-right">
                        <g:pageProperty name="page.nav" />
                        <div class="pull-right"  style="margin-right: 10px">
                        <sec:ifLoggedIn>
                            <li class="">
                            %{--<li class="navbar-link">--}%
                                <a class="navbar-item"><sec:username /></a>
                            </li>
                            <li class="navbar-link">
                                <g:link class="nav-link" controller="logout">Logout</g:link>
                                %{-- for old version only now E: tag does not exist! <g:remoteLink class="logout" controller="logout">${message(code: 'springSecurity.logout.link')}</g:remoteLink> --}%
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
        </div>
        <div class="row">
            <div class="col-sm-2 pull-left" style="padding: 0;">
                <div id="projTree" >
                    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: [title: 'myExperiment']]" />
                </div>
            </div>
            <div class="col-sm-10">
                <g:layoutBody/>
            </div>
        </div>
    </div>%{--wrapper--}%
    <div class="footer" role="contentinfo"></div>
    <div id="screen-locker" style="display: none;"></div>
    <div id="spinner" class="spinner" style="display:none;">
        <g:message code="spinner.alt" default="Loading&hellip;"/>
    </div>
    <asset:javascript src="application.js"/>
</body>
</html>
