<content tag="nav">
  %{--<sec:ifAnyGranted roles="ROLE_NewUser,ROLE_User,ROLE_Admin,ROLE_Administrator">--}%
    %{--<li class="dropdown">--}%
      %{--<a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"--}%
         %{--aria-expanded="false">Users Area <span class="caret"></span></a>--}%
      %{--<ul class="dropdown-menu">--}%
        %{--<g:link controller="project" action="list">Projects</g:link>--}%
      %{--</ul>--}%
    %{--</li>--}%
  %{--</sec:ifAnyGranted>--}%
  <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"
         aria-expanded="false">Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        %{--<sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ManageUsers">--}%
          %{--<li><g:link controller="experiment" action="manageUsers">Manage Users</g:link></li>--}%
        %{--</sec:ifAnyGranted>--}%
        <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_UsersList">
          <li><g:link controller="user" action="newUsers">New Users</g:link></li>
        </sec:ifAnyGranted>
      </ul>
    </li>
  </sec:ifAnyGranted>
</content>