<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<content tag="nav">
%{--<li class="dropdown">
    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Application Status <span class="caret"></span></a>
    <ul class="dropdown-menu">
        <li><a href="#">Environment: ${grails.util.Environment.current.name}</a></li>
        <li><a href="#">App profile: ${grailsApplication.config.grails?.profile}</a></li>
        <li><a href="#">App version:
            <g:meta name="info.app.version"/></a>
        </li>
        <li role="separator" class="divider"></li>
        <li><a href="#">Grails version:
            <g:meta name="info.app.grailsVersion"/></a>
        </li>
        <li><a href="#">Groovy version: ${GroovySystem.getVersion()}</a></li>
        <li><a href="#">JVM version: ${System.getProperty('java.version')}</a></li>
        <li role="separator" class="divider"></li>
        <li><a href="#">Reloading active: ${grails.util.Environment.reloadingAgentEnabled}</a></li>
    </ul>
</li>--}%
%{--<li class="dropdown">
    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Artefacts <span class="caret"></span></a>
    <ul class="dropdown-menu">
        <li><a href="#">Controllers: ${grailsApplication.controllerClasses.size()}</a></li>
        <li><a href="#">Domains: ${grailsApplication.domainClasses.size()}</a></li>
        <li><a href="#">Services: ${grailsApplication.serviceClasses.size()}</a></li>
        <li><a href="#">Tag Libraries: ${grailsApplication.tagLibClasses.size()}</a></li>
    </ul>
</li>--}%
%{--<sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
        <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"><i class="fa fa-gear"></i> Settings <span class="caret"></span></a>
        <ul class="dropdown-menu">
            --}%%{--<g:each var="plugin" in="${applicationContext.getBean('pluginManager').allPlugins}">
                <li><a href="#">${plugin.name} - ${plugin.version}</a></li>
            </g:each>--}%%{--
            <sec:ifAnyGranted roles="ROLE_Administrator">
                --}%%{--<g:link controller="user">Manage Users</g:link>--}%%{--
            </sec:ifAnyGranted>
            <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
                --}%%{--<g:link controller="user" action="newUsers">New Users</g:link>--}%%{--
            </sec:ifAnyGranted>
        </ul>
    </li>
</sec:ifAnyGranted>--}%
%{--
<sec:ifLoggedIn>
    -- <li><a><sec:username /></a></li> --
    -- <li><g:link controller="logout">Logout</g:link></li> --
    <p class="navbar-text" style="color: white">Logged in as: <sec:username /></p>
    <g:link controller="logout" class="navbar-text">Logout</g:link>
</sec:ifLoggedIn>
<sec:ifNotLoggedIn>
    <li><g:link controller='login' action='auth'>Login</g:link></li>
</sec:ifNotLoggedIn>
--}%
</content>
%{--<a href="#list-module" class="skip" tabindex="-1"><g:message code="default.link.skip.label"
                                                             default="Skip to content&hellip;"/></a>

<div class="nav" role="navigation">
  <ul>
    <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
    <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]"/></g:link></li>
  </ul>
</div>--}%

  <div id="list-module" class="container" role="main">
    <h1 class="page-header"><g:message code="default.list.label" args="[entityName]"/></h1>
    <g:if test="${flash.message}">
      <div class="message" role="status">${flash.message}</div>
    </g:if>
    <div class="row">
      <div class="col-sm-12">
        <f:table collection="${moduleList}" properties="${['label','name', 'server']}"/>
        <div class="pagination">
          <g:paginate total="${moduleCount ?: 0}"/>
        </div>
      </div>
    </div>
  </div>
</body>
</html>