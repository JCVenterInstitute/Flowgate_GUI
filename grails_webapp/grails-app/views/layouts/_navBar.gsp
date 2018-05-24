<nav class="navbar navbar-dark bg-dark" role="navigation">
  <div class="container">
    <!-- Brand and toggle get grouped for better mobile display -->
    <div class="navbar-header">
      <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#flowgate-navbar-collapse" aria-expanded="false">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      <a class="navbar-brand" href="${createLink(uri: '/')}"><img src="${assetPath(src: 'logo-dark.png')}" style="height: 40px;" class="img-fluid"></a>
    </div>

    <!-- Collect the nav links, forms, and other content for toggling -->
    <div class="collapse navbar-collapse" id="flowgate-navbar-collapse">
      <ul class="nav navbar-nav navbar-right">
        <sec:ifLoggedIn>
          <li class="nav-item">
            <a class="nav-link" href="${createLink(uri: '/project/list')}">Projects</a>
          </li>
          <g:pageProperty name="page.nav" />
          <li class="dropdown" style="min-width: 100px;">
            <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true"
               aria-expanded="false"><i class="fa fa-user" style="margin-right: 5px;"></i> <sec:username/> <span class="caret"></span></a>
            <ul class="dropdown-menu">
              <li><g:link controller="logout">Logout</g:link></li>
            </ul>
          </li>
        </sec:ifLoggedIn>
        <sec:ifNotLoggedIn>
          <li id="navbar-login" class="navbar-item">
            <g:link controller="logout" class="nav-link">Login</g:link>
          </li>
        </sec:ifNotLoggedIn>
      </ul>
    </div><!-- /.navbar-collapse -->
  </div><!-- /.container-fluid -->
</nav>
