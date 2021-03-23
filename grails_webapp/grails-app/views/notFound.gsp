<!doctype html>
<html>
<head>
  <title>Page Not Found</title>
  <meta name="layout" content="main">
  <g:if env="development"><asset:stylesheet src="errors.css"/></g:if>
</head>

<body>
<g:if env="development">
  <ul class="errors">
    <li>Error: Page Not Found (404)</li>
    <li>Path: ${request.forwardURI}</li>
  </ul>
</g:if>
<g:else>
  <div class="center-align">
    <h2>404</h2>

    <h6>
      Oops, the page you're looking for doesn't exist.
    </h6>

    <div class="input-field col s12">
      <a class="btn" href="${createLink(uri: '/')}">Go to homepage</a>
    </div>
  </div>
</g:else>
</body>
</html>
