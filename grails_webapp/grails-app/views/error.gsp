<!doctype html>
<html>
<head>
  <title><g:if env="development">Grails Runtime Exception</g:if><g:else>Error</g:else></title>
  <meta name="layout" content="main">
  <g:if env="development"><asset:stylesheet src="errors.css"/></g:if>
</head>

<body>
<g:if env="development">
  <g:if test="${Throwable.isInstance(exception)}">
    <g:renderException exception="${exception}"/>
  </g:if>
  <g:elseif test="${request.getAttribute('javax.servlet.error.exception')}">
    <g:renderException exception="${request.getAttribute('javax.servlet.error.exception')}"/>
  </g:elseif>
  <g:else>
    <ul class="errors">
      <li>An error has occurred</li>
      <li>Exception: ${exception}</li>
      <li>Message: ${message}</li>
      <li>Path: ${path}</li>
    </ul>
  </g:else>
</g:if>
<g:else>
  <div class="center-align">
    <h2>${request.getAttribute('javax.servlet.error.status_code')}</h2>

    <h6>
      An error has occurred
    </h6>

    <div class="input-field col s12">
      <a class="btn" href="${createLink(uri: '/')}">Go to homepage</a>
    </div>
  </div>
</g:else>
</body>
</html>
