<%@ page import="grails.converters.JSON" %>
<strong>Results</strong>
<br/>
<div class="btn btn-default" onclick="alert('not implemented yet');" style="display: none">Download Results as Zip-file</div>
<br/>
  <iframe name="htmlReport" src="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" height=1200 style="width: 100%" sandbox="allow-same-origin allow-scripts allow-forms"></iframe>
  %{--<fg:render style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>--}%
  %{--<fg:render2 style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>--}%
  %{--<fg:renderHtmlFile fileName="report.html" />--}%
  %{--<fg:renderHtmlFile fileName="notebookDiv.html" />--}%
<br/>
<br/>