<asset:javascript src="jquery-2.2.0.min.js"/>
<hr/>
<div class="row">
  <div  style="padding-left:30px;" >
    %{--<div class="btn btn-info" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">newHtml2</div>--}%
    <div class="btn btn-info" data-toggle="modal" data-target="#resultFilesModal">File List</div>
  </div>

  %{--
  <g:set var="colNo" value="${9}"/>
  --}%%{-- -- <g:each in="${jobResult.outputFiles.findAll { (it.kind.contains('png') || it.kind.contains('jpg') || it.kind.contains('jpeg')) }}" var="outputFile" status="i"> -- --}%%{--
  <g:each in="${jobResult.outputFiles}" var="outputFile" status="i">
    <g:if test="${(i + 1) % colNo == 0}">
      <div class="row" style="max-width: 100%">
    </g:if>
    <div class="col-sm-2}" >
      <p class="">${outputFile?.path}</p>
      --}%%{-- -- <p style="padding-left: 10px"><img src="${outputFile?.link?.href}" alt="${outputFile?.filename}" width="70%"/></p> -- --}%%{--
      <p style="padding-left: 35%">
        <a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}">
        --}%%{-- -- ${outputFile?.path}&nbsp; -- --}%%{--<i class="fa fa-floppy-o fa-lg"></i>
        </a>
      </p>
      --}%%{-- -- <p><a href="${outputFile?.link?.href}" target="_blank">View On Server&nbsp;<i class="glyphicon glyphicon-eye-open"></i></a></p> -- --}%%{--
    </div>
    <g:if test="${(i + 1) % colNo == 0}">
      </div>
      <br/>
      <hr/>
      <br/>
    </g:if>
    --}%
  %{--</g:each>--}%

  <div class="modal fade in" id="resultFilesModal" role="dialog">
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-body custom-height-modal">

          <g:each in="${jobResult.outputFiles}" var="outputFile" status="i">
            %{--<p class="">${outputFile?.path}</p>--}%
            <li class="">${outputFile?.path}
              <a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}">
               %{---- ${outputFile?.path}&nbsp; -- --}%dwnld&nbsp;<i class="fa fa-floppy-o fa-lg"></i>
              </a>&nbsp;/&nbsp;
            %{--<p><a href="${outputFile?.link?.href}" target="_blank">View On Server&nbsp;<i class="glyphicon glyphicon-eye-open"></i></a></p>--}%
              <a href="${outputFile?.link?.href}" target="_blank">View On Server&nbsp;<i class="glyphicon glyphicon-eye-open"></i></a>
            </li>
          </g:each>

        </div>
      </div>
    </div>
  </div>
</div>
<hr/>