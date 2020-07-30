%{--
<div class="modal fade" tabindex="-1" id="resultModal-${bean?.jobNumber}" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">
  <div class="modal-dialog modal-lg">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <div id="myModalLabel">
          <h4 class="modal-title" style="text-align: left;"><div class="text-center">Results</div></h4>
        </div>
      </div>

      <div class="modal-body custom-height-modal">
        <object style="width: 100%;height:100%;" data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber])}" ></object>
      </div>

      <div class="modal-footer">
        <button type="button" id="aux${moduleParam?.id}SaveBtn" style="display: none" class="left btn btn-success" >Save</button>
        <button type="button" id="aux${moduleParam?.id}CloseBtn" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>
--}%
%{--<div class="modal fade" tabindex="-1" id="resultModal-${bean.jobNumber}" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">--}%
%{--<div class="modal fade" tabindex="-1" id="resultModal-256" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">--}%
<div class="modal fade" tabindex="-1" id="resultModal-${jobNumber}" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">
  <div class="modal-dialog modal-lg">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <div id="myModalLabel">
          <h4 class="modal-title" style="text-align: left;"><div class="text-center">Results</div></h4>
        </div>
      </div>

      <div class="modal-body custom-height-modal">
        %{-- -- TODO reactivate the loading -- --}%
        %{-- -- <div class="loading" id="loadingMessage" style="position:absolute; top: 50%; left: 50%">loading...</div> -- --}%
        %{-- -- <object style="width: 100%;height:800px;" data="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}" ></object> -- --}%


        <object style="width: 100%;height:100%;" data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: id, jobNr: jobNumber])}" ></object>


        %{-- --
          <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px" scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0"></iframe>
        -- --}%
      </div>

      <div class="modal-footer">
        <button type="button" id="resultModal-${jobNumber}SaveBtn" style="display: none" class="left btn btn-success" >Save</button>
        <button type="button" id="resultModal-${jobNumber}CloseBtn" onclick="checkForMissedEvents()" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>
