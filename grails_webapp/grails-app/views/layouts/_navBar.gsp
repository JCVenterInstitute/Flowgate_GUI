<nav>
  <ul id="dropdown-user-menu" class="dropdown-content">
    <li><g:link controller="logout">Logout</g:link></li>
  </ul>
  <ul id="dropdown-user-menu-mobile" class="dropdown-content">
    <li><g:link controller="logout">Logout</g:link></li>
  </ul>

  <div class="nav-wrapper row">
    <div class="col s12">
      <a href="${createLink(uri: '/')}" class="brand-logo">FlowGate</a>
      <a href="#" data-target="mobile-nav" class="sidenav-trigger right"><i class="material-icons">menu</i></a>
      <ul id="nav-mobile" class="right hide-on-med-and-down">
        <sec:ifLoggedIn>
          <li id="nav-projects"><a href="${createLink(uri: '/project/list')}">Projects</a></li>
          <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
            <li id="nav-transformation"><g:link controller="transformationParameter" action="index">Transformation</g:link></li>
            <li id="nav-analysis"><g:link controller="analysisServer" action="index">Analysis Servers</g:link></li>
            <li id="nav-modules"><g:link controller="module" action="index">Modules</g:link></li>
            <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_UsersList">
              <li id="nav-users"><g:link controller="user" action="list">Users</g:link></li>
            </sec:ifAnyGranted>
          </sec:ifAnyGranted>
          <li id="nav-about"><a href="${createLink(uri: '/about')}">About</a></li>
          <li><a class="dropdown-trigger" href="#!" data-target="dropdown-user-menu">
            <sec:username/><i class="material-icons right">arrow_drop_down</i>
          </a></li>
        </sec:ifLoggedIn>
      </ul>
    </div>
  </div>
</nav>

<ul class="sidenav" id="mobile-nav">
  <sec:ifLoggedIn>
    <li id="nav-projects"><a href="${createLink(uri: '/project/list')}">Projects</a></li>
    <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
      <li id="nav-transformation"><g:link controller="transformationParameter" action="index">Transformation</g:link></li>
      <li id="nav-analysis"><g:link controller="analysisServer" action="index">Analysis Servers</g:link></li>
      <li id="nav-modules"><g:link controller="module" action="index">Modules</g:link></li>
      <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_UsersList">
        <li id="nav-users"><g:link controller="user" action="list">Users</g:link></li>
      </sec:ifAnyGranted>
    </sec:ifAnyGranted>
    <li id="nav-about"><a href="${createLink(uri: '/about')}">About</a></li>
    <li><a class="dropdown-trigger" href="#!" data-target="dropdown-user-menu-mobile">
      <sec:username/><i class="material-icons right">arrow_drop_down</i>
    </a></li>
  </sec:ifLoggedIn>
</ul>

<script>
  document.addEventListener('DOMContentLoaded', function() {
    var sideNav = document.querySelectorAll('.sidenav');
    M.Sidenav.init(sideNav, {edge:'right'});
  });
</script>
