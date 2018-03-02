<div class="navbar navbar-default navbar-static-top" role="navigation">
    %{--<div class="container">--}%
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
                </i>%{--<g:if env="production">flowgate-prod</g:if><g:else>flowgate</g:else>--}%
            </a>
        </div>
        <div class="navbar-collapse collapse" aria-expanded="false" style="height: 0.8px;">
            <ul class="nav navbar-nav navbar-right pull-right">
                <g:pageProperty name="page.nav" />
                <div class="pull-right"  style="margin-right: 10px">
                    <sec:ifLoggedIn>
                        <li class="">
                            %{--<li class="navbar-link">--}%
                            <div class="nav-item" style="padding-top: 4px;color: white" ><span class="glyphicon glyphicon-user"></span>&nbsp;<sec:username /></div>
                        </li>
                        <li class="navbar-link">
                            <g:link controller="logout">Logout</g:link>
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
    %{--</div>--}%
</div>
