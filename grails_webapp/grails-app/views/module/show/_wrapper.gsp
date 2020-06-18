<%@ page defaultCodec="html" %>
<g:if test="${value != null && value != ''}">
  <div>
    <label>${label}</label>
    <g:if test="${property == 'descript'}">
      <pre style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen-Sans, Ubuntu, Cantarell, 'Helvetica Neue', sans-serif;">${value}</pre>
    </g:if>
    <g:else>
      <p>${value}</p>
    </g:else>
  </div>
</g:if>