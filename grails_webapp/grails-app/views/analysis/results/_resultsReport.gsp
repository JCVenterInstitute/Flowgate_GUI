<%@ page import="grails.converters.JSON" %>
<strong>Results</strong>
<br/>
<div class="btn btn-default" onclick="alert('not implemented yet');">Download Results as Zip-file</div>
<br/>
  %{--<fg:render style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>--}%
  <fg:render2 style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>
  %{--<fg:renderHtmlFile fileName="report.html" />--}%
  %{--<fg:renderHtmlFile fileName="notebookDiv.html" />--}%
<br/>
<br/>