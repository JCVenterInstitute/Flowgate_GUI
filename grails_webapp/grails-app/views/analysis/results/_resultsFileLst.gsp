<hr/>
<div class="row">
  <div  style="padding-left:30px;" >
    <div class="btn btn-info" data-toggle="modal" data-target="#resultFilesModal">File List</div>
  </div>
  <div class="modal fade in" id="resultFilesModal" role="dialog">
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-body custom-height-modal">

          <g:each in="${jobResult.outputFiles}" var="outputFile" status="i">
            <li class="">${outputFile?.path}
              <a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}">
               dwnld&nbsp;<i class="fa fa-floppy-o fa-lg"></i>
              </a>&nbsp;/&nbsp;
              <a href="${outputFile?.link?.href}" target="_blank">View On Server&nbsp;<i class="glyphicon glyphicon-eye-open"></i></a>
            </li>
          </g:each>

        </div>
      </div>
    </div>
  </div>
</div>
<hr/>