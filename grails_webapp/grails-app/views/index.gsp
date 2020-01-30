<!DOCTYPE html>
<g:set var='securityConfig' value='${applicationContext.springSecurityService.securityConfig}'/>
<html lang="en">
<head>
  <meta name="layout" content="ofg"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico"/>
  <title><g:message code="landingPage.title.label" default="FlowGate"/></title>
  <asset:stylesheet src="signin.css"/>
</head>

<body>
<div class="container">
  <div class="row valign-wrapper">
    <div class="col l8 m8 s12">
      <div class="carousel carousel-slider" style="height: 470px">
        <a class="carousel-item" href="#one!"><img src="${assetPath(src: 'landingPageCarousel2/NewFigure1.png')}"></a>
        <a class="carousel-item" href="#two!"><img src="${assetPath(src: 'landingPageCarousel2/NewFigure2.png')}"></a>
        <a class="carousel-item" href="#three!"><img src="${assetPath(src: 'landingPageCarousel2/NewFigure3.png')}"></a>
        <a class="carousel-item" href="#four!"><img src="${assetPath(src: 'landingPageCarousel2/NewFigure4.png')}"></a>
      </div>
    </div>

    <div class="col l4 m4 s12 center-align">
      %{--<img src="${assetPath(src: 'logo-light.png')}" alt="FlowGate" height="72">--}%
      <s2ui:form type='login' focus='username'>
        <div class="row mb-0">
          <div class="input-field col s12">
            <h5 class="ml-4">Sign in to FlowGate</h5>
          </div>
        </div>

        <div class="row mb-0">
          <div class="input-field col s12">
            <i class="material-icons prefix pt-2">person_outline</i>
            <input id="username" type="text" name="${securityConfig.apf.usernameParameter}">
            <label for="username" class="center-align">Username</label>
          </div>
        </div>

        <div class="row mb-0">
          <div class="input-field col s12">
            <i class="material-icons prefix pt-2">lock_outline</i>
            <input id="password" type="password" name="${securityConfig.apf.passwordParameter}">
            <label for="password">Password</label>
          </div>
        </div>

        <div class="row mb-0">
          <div class="input-field col s12">
            <button type="submit" class="btn waves-effect waves-light border-round col s12">${g.message(code: 'spring.security.ui.login.signin')}</button>
          </div>
        </div>

        <g:if test="${flash.message}">
          <div class="row mb-0">
            <div class="col s12">
              <div class="card-panel error p-2">
                <span class="white-text">${flash.message}</span>
              </div>
            </div>
          </div>
        </g:if>

        <div class="row mb-0">
          <div class="col s6 m6 l6">
            <p class="margin medium-small"><s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/></p>
          </div>

          <div class="col s6 m6 l6">
            <p class="margin right-align medium-small"><g:link controller="register" action="forgotPassword"><g:message code="spring.security.ui.login.forgotPw" default="Reset password"/></g:link></p>
          </div>
        </div>

        <div class="row mb-0">
          <div class="col s12">
            <p class="margin medium-small">FlowGate &copy; 2019 | <g:link uri="/about"><g:message code="about.link.label" default="About"/></g:link> | <g:link
                uri="mailto:${grailsApplication.config.adminEmailAddr}">Contact Us</g:link></p>
          </div>
        </div>
      </s2ui:form>
    </div>
  </div>
</div>

%{--<div class="container">
  <div class="row">
    <div class="col-sm-4">
      <s2ui:form type='login' focus='username' class="form-signin">
        <img class="mb-4 img-fluid" src="${assetPath(src: 'logo-light.png')}" alt="Flow Gate" height="72">

        <h1 class="h3 mb-3 font-weight-normal"><g:message code='spring.security.ui.login.signin'/></h1>
        <label for="username" class="sr-only">Username</label>
        <input type="text" name="${securityConfig.apf.usernameParameter}" id="username" placeholder="Username" class="form-control" required="" autofocus=""/>
        <label for="password" class="sr-only">Password</label>
        <input type="password" name="${securityConfig.apf.passwordParameter}" id="password" placeholder="Password" class="form-control" required=""/>

        <div class="mb-3"></div>
        <button class="btn btn-lg btn-primary btn-block" type="submit">${g.message(code: 'spring.security.ui.login.login')}</button>

        <div>
          <s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/> or <g:link controller="register"
                                                                                                                                   action="forgotPassword"><g:message
                code="spring.security.ui.login.forgotPw" default="Reset password"/></g:link>
        </div>

        <p class="mt-5 mb-3 text-muted">FlowGate &copy; 2019 | <g:link uri="/about"><g:message code="about.link.label" default="About"/></g:link> | <g:link
            uri="mailto:${grailsApplication.config.adminEmailAddr}">Contact Us</g:link></p>

        <div class="g-recaptcha" data-sitekey="6LevxvUSAAAAAKQvzYLd-oJmUXpkPGMw9nFq-0GH"></div>
      </s2ui:form>
    </div>
  </div>
</div>--}%
<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('.carousel.carousel-slider');
    M.Carousel.init(elems, {
      fullWidth: true,
      indicators: true
    });
  });
</script>
</body>
</html>