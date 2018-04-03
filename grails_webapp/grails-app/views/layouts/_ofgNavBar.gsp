<div class="navbar navbar-default navbar-static-top" role="navigation">
  %{--<div class="container">--}%
  <div class="navbar-header">
    <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
      <span class="sr-only">Toggle navigation</span>
      <span class="icon-bar"></span>
      <span class="icon-bar"></span>
      <span class="icon-bar"></span>
    </button>
    <a class="navbar-brand" href="${createLink(uri: '/about')}">
      <i class="flowgateLogo"><asset:image src="flowgate2.png" height="50"/></i>
    </a>
  </div>

  <div class="navbar-collapse collapse" aria-expanded="false" style="height: 0.8px;">
    <ul class="nav navbar-nav navbar-right pull-right">
      <li class="navbar-link">
        <g:link uri="/about"><g:message code="about.link.label" default="About"/></g:link>
      </li>
    </ul>
  </div>
  %{--</div>--}%
</div>
