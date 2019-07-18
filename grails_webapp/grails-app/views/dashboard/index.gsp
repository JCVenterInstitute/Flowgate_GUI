<!doctype html>
<html>
<head>
  <meta name="layout" content="main"/>
  %{--<title>fgate</title>--}%
  <asset:link rel="icon" href="favicon.ico" type="image/x-ico"/>
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
         aria-expanded="false">Users Area <span class="caret"></span></a>
      <ul class="dropdown-menu">
        <li><a href="#"><g:link controller="asyncProject" action="index"></g:link></a></li>
        %{--<g:link controller="asyncProject" action="index" >Async Projects</g:link>--}%
        <g:link controller="project" action="index">Projects</g:link>
        <g:link controller="project" action="list">Projs. List</g:link>
        <g:link controller="experiment" action="index">Experiments</g:link>

        %{--<li><a href="#">Domains: ${grailsApplication.domainClasses.size()}</a></li>--}%
        %{--<li><a href="#">Services: ${grailsApplication.serviceClasses.size()}</a></li>--}%
        %{--<li><a href="#">Tag Libraries: ${grailsApplication.tagLibClasses.size()}</a></li>--}%
      </ul>
    </li>
  </sec:ifAnyGranted>
  <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"
         aria-expanded="false"><i class="fa fa-gear"></i> Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        %{--<g:each var="plugin" in="${applicationContext.getBean('pluginManager').allPlugins}">
            <li><a href="#">${plugin.name} - ${plugin.version}</a></li>
        </g:each>--}%
        <sec:ifAnyGranted roles="ROLE_Administrator">
          <g:link controller="user">Manage Users</g:link>
        </sec:ifAnyGranted>
        <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
          <g:link controller="user" action="list">Users</g:link>
        </sec:ifAnyGranted>
      </ul>
    </li>
  </sec:ifAnyGranted>
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

<h1 class="text-center">dashboard</h1>
<br/>

<div id="content" role="main">
  <section class="row colset-2-its">
    <sec:ifAnyGranted roles="ROLE_Administrator">ROLE_Administrator</sec:ifAnyGranted>
    <sec:ifAnyGranted roles="ROLE_Admin">ROLE_Admin</sec:ifAnyGranted>
    <sec:ifAnyGranted roles="ROLE_User">ROLE_User</sec:ifAnyGranted>
    <sec:ifAnyGranted roles="ROLE_NewUser">ROLE_NewUser</sec:ifAnyGranted>
    <sec:ifAnyGranted roles="ROLE_Guest">ROLE_Guest</sec:ifAnyGranted>

    <sec:ifAnyGranted roles="ROLE_Administrator">
      <div id="controllers" role="navigation">
        <h2>Available Controllers:</h2>
        <ul>
          <g:each var="c" in="${grailsApplication.controllerClasses.sort { it.fullName }}">
            <li class="controller">
              <g:link controller="${c.logicalPropertyName}">${c.fullName}</g:link>
              &nbsp;${c.logicalPropertyName}
              &nbsp;${c.fullName}
            </li>
          </g:each>
        </ul>
      </div>
    </sec:ifAnyGranted>
  </section>
</div>

</body>
</html>
