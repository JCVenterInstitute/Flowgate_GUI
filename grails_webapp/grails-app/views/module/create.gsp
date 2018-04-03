<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
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

  <sec:ifAnyGranted roles="ROLE_NewUser,ROLE_User,ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"
         aria-expanded="false">Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        %{--<li><a href="#"><g:link controller="asyncProject" action="index" ></g:link></a></li>--}%
        %{--<g:link controller="asyncProject" action="index" >Async Projects</g:link>--}%
        <g:link controller="analysisServer" action="index">Analysis Servers</g:link>
        <g:link controller="module" action="index">Moduls List</g:link>
        %{--<g:link controller="experiment" action="index" >Experiments</g:link>--}%

        %{--<li><a href="#">Domains: ${grailsApplication.domainClasses.size()}</a></li>--}%
        %{--<li><a href="#">Services: ${grailsApplication.serviceClasses.size()}</a></li>--}%
        %{--<li><a href="#">Tag Libraries: ${grailsApplication.tagLibClasses.size()}</a></li>--}%
      </ul>
    </li>
  </sec:ifAnyGranted>
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
%{--<a href="#create-module" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%

<div class="nav" role="navigation">
  %{--
  <ul>
    <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
    <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
  </ul>
  --}%
</div>

<div id="create-module" class="content scaffold-create" role="main">
  <h1><g:message code="default.create.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
  <g:hasErrors bean="${this.module}">
    <ul class="errors" role="alert">
      <g:eachError bean="${this.module}" var="error">
        <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form action="save">
    <fieldset class="form">
      <f:all bean="module"/>
    </fieldset>
    <fieldset class="buttons">
      <g:submitButton name="create" class="save" value="${message(code: 'default.button.create.label', default: 'Create')}"/>
    </fieldset>
  </g:form>
</div>
</body>
</html>
