<!DOCTYPE html>
<g:set var='securityConfig' value='${applicationContext.springSecurityService.securityConfig}'/>
<html lang="en">
<head>
  <meta name="layout" content="ofg"/>
  <asset:link rel="icon" href="f.ico" type="image/x-ico" />
  <title><g:message code="landingPage.title.label" default="FlowGate"/></title>
  <asset:stylesheet src="signin.css"/>
</head>

<body>
<div class="container">
  <div class="row">
    <div class="col-sm-8">
      <div id="flowgateCarousel" class="carousel slide" data-ride="carousel">
        <ol class="carousel-indicators">
          <li data-target="#flowgateCarousel" data-slide-to="0" class="active"></li>
          <li data-target="#flowgateCarousel" data-slide-to="1"></li>
          <li data-target="#flowgateCarousel" data-slide-to="2"></li>
          <li data-target="#flowgateCarousel" data-slide-to="3"></li>
          <li data-target="#flowgateCarousel" data-slide-to="4"></li>
          <li data-target="#flowgateCarousel" data-slide-to="5"></li>
        </ol>
        <div class="carousel-inner">
          <div class="item active">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure1.png')}">
          </div>
          <div class="item">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure2.png')}">
          </div>
          <div class="item">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure3.png')}">
          </div>
          <div class="item">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure4.png')}">
          </div>
          <div class="item">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure5.png')}">
          </div>
          <div class="item">
            <img class="d-block w-100 img-fluid" src="${assetPath(src: 'landingPageCarousel2/NewFigure6.png')}">
          </div>
        </div>
        <a class="left carousel-control" href="#flowgateCarousel" role="button" data-slide="prev">
          <i class="fa fa-chevron-left"></i>
          <span class="sr-only">Previous</span>
        </a>
        <a class="right carousel-control" href="#flowgateCarousel" role="button" data-slide="next">
          <i class="fa fa-chevron-right"></i>
          <span class="sr-only">Next</span>
        </a>
      </div>
    </div>
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
          <s2ui:linkButton elementId='register' controller='register' messageCode='spring.security.ui.login.register'/> or <g:link controller="register" action="forgotPassword"><g:message code="spring.security.ui.login.forgotPw" default="Reset password"/></g:link>
        </div>
        <p class="mt-5 mb-3 text-muted">FlowGate &copy; 2018 | <g:link uri="/about" ><g:message code="about.link.label" default="About" /></g:link> | <g:link uri="mailto:${grailsApplication.config.adminEmailAddr}" >Contact Us</g:link></p>
      </s2ui:form>
    </div>
  </div>
</div>
</body>
</html>