<%@ page import="grails.converters.JSON" %>
<dt>Available Reports:</dt>
<dd><g:each in="${jobResult.outputFiles.findAll { (it.kind.contains('html')) }}" var="outputFile" status="i">
  ${outputFile?.path} <a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}">
    %{---- ${outputFile?.path}&nbsp; -- --}%dwnld&nbsp;<i class="fa fa-floppy-o fa-lg"></i>
  </a><g:if test="${i != jobResult.outputFiles.findAll { (it.kind.contains('html')) }.size() - 1}"></br></g:if>
</g:each></dd>
<dt>Job Results Archive:</dt>
<dd><g:each in="${jobResult.outputFiles.findAll { (it.link.name.contains('Job.')) }}" var="outputFile" status="i">
  ${outputFile?.path} <a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}">
    %{---- ${outputFile?.path}&nbsp; -- --}%dwnld&nbsp;<i class="fa fa-floppy-o fa-lg"></i>
  </a><g:if test="${i != jobResult.outputFiles.findAll { (it.kind.contains('html')) }.size() - 1}"></br></g:if>
</g:each></dd>
%{--<div class="btn btn-default" onclick="alert('not implemented yet');">Download Results as Zip-file</div>--}%
%{--<iframe name="htmlReport" src="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" height=1200 style="width: 100%" sandbox="allow-same-origin allow-scripts allow-forms"></iframe>--}%
%{--<fg:render style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>--}%
%{--<fg:render2 style="z-index: -100" href="${jobResult.outputFiles.find{it.link.name.contains('Reports/')}.link.href}" analysisServerId="${this?.analysis?.module?.server?.id}"/>--}%
%{--<fg:renderHtmlFile fileName="report.html" />--}%
%{--<fg:renderHtmlFile fileName="notebookDiv.html" />--}%
