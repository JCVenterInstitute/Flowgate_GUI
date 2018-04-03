<content tag="nav">
  <sec:ifAnyGranted roles="ROLE_NewUser,ROLE_User,ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"
         aria-expanded="false">Users Area <span class="caret"></span></a>
      <ul class="dropdown-menu">
        %{--<li><a href="#"><g:link controller="asyncProject" action="index" ></g:link></a></li>--}%
        %{--<g:link controller="asyncProject" action="index" >Async Projects</g:link>--}%
        <g:link controller="project" action="list">Projects</g:link>
        %{--<g:link controller="project" action="list" >Projs. List</g:link>--}%
        %{--<g:link controller="experiment" action="index" >Experiments</g:link>--}%

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
        <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ManageUsers">
          <g:link controller="experiment" action="manageUsers">Manage Users</g:link>
        </sec:ifAnyGranted>
        <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_UsersList">
          <g:link controller="user" action="newUsers">New Users</g:link>
        </sec:ifAnyGranted>
      </ul>
    </li>
  </sec:ifAnyGranted>
</content>