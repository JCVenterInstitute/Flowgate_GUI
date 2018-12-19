<table id="analysis-table" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <th>Task Name</th>
    <th>Task Result Files</th>
    <th>Task Status</th>
    <th>Submitted on</th>
    <th>Creator</th>
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <tr>
      <g:each in="${['analysisName', 'taskResultFiles', 'analysisStatus', 'timestamp', 'user.username']}" var="p" status="j">
        <g:if test="${j == 0}">
          <td>
          %{-- TODO when results ready redirect to show results page --}%
            <g:if test="${bean?.analysisStatus != 3 && bean?.analysisStatus != 4}">
              <g:link action="edit" resource="${bean}">
                <f:display bean="${bean}"
                           property="analysisName"
                           displayStyle="${displayStyle ?: 'table'}"/>%{-- &nbsp;Job=${bean.jobNumber} --}%
              </g:link>
            </g:if>
            <g:else>
              %{--<g:if test="${bean?.analysisStatus == 4}">--}%
                %{--<div class="div-as-link" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">--}%
                  %{--<f:display bean="${bean}"--}%
                             %{--property="analysisName"--}%
                             %{--displayStyle="${displayStyle ?: 'table'}"/>--}%%{--&nbsp;Job=${bean.jobNumber}--}%
                %{--</div>--}%
              %{--</g:if>--}%
              %{--<g:else>--}%
                <div class="div-as-link" style="cursor: pointer;" data-toggle="modal" data-target="#resultModal-${bean.jobNumber}">
                  <f:display bean="${bean}"
                             property="analysisName"
                             displayStyle="${displayStyle ?: 'table'}"/>%{-- &nbsp;Job=${bean.jobNumber} --}%
                </div>
              %{--</g:else>--}%
            </g:else>
          </td>
        </g:if>
        <g:else>
          <g:if test="${j == 1}">
            <td>
            %{-- TODO when results ready redirect to show results page --}%
              <g:if test="${bean?.analysisStatus != 3 && bean?.analysisStatus != 4}">
                <g:link action="edit" resource="${bean}">
                  <f:display bean="${bean}"
                             property="analysisName"
                             displayStyle="${displayStyle ?: 'table'}"/>%{-- &nbsp;Job=${bean.jobNumber} --}%
                </g:link>
              </g:if>
              <g:else>
                <g:if test="${bean?.analysisStatus == 4}">
                  <div class="div-as-link" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">
                    <f:display bean="${bean}"
                               property="analysisName"
                               displayStyle="${displayStyle ?: 'table'}"/>%{--&nbsp;Job=${bean.jobNumber}--}%
                  </div>
                </g:if>
                <g:else>
                  <g:link action="showResults" resource="${bean}">
                    <div class="div-as-link form-control-plaintext">
                    ${bean.analysisName.take(20)}... - result Files
                    %{--<f:display bean="${bean}"--}%
                               %{--property="analysisName"--}%
                               %{--displayStyle="${displayStyle ?: 'table'}"/>--}%%{-- &nbsp;Job=${bean.jobNumber} --}%
                    </div>
                  </g:link>
                </g:else>
              </g:else>
            </td>
          </g:if>
          <g:else>
            <td <g:if test="${p == 'timestamp'}">style="vertical-align: inherit;"</g:if>>
              <g:if test="${p == 'analysisStatus'}">
                <div id="job-${bean.jobNumber}" class="form-control-plaintext">
                  %{--jobnr${bean.jobNumber}--}%
                  %{--<div class="div-as-link" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">--}%
                  <div class="div-as-link" data-toggle="modal" data-target="#resultModal-${bean.jobNumber}">
                    <i class=" fa fa-circle"
                       style="color: ${bean.analysisStatus == 1 ? 'lightgrey' : bean.analysisStatus == 2 ? 'orange' : bean.analysisStatus == 3 ? 'lawngreen' : bean.analysisStatus == -1 ? 'red' : 'brown'}"></i>
                    %{--&nbsp;&nbsp;&nbsp;--}%
                    ${bean.analysisStatus == 1 ? 'init' : bean.analysisStatus == 2 ? 'pending' : bean.analysisStatus == 3 ? 'results ready' : bean.analysisStatus == -1 ? 'error' : 'done'}
                  </div>
                </div>
                <div class="modal fade" tabindex="-1" id="resultModal-${bean.jobNumber}" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">
                  <div class="modal-dialog modal-lg">
                    <div class="modal-content">
                      <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <div id="myModalLabel">
                          <h4 class="modal-title" style="text-align: left;"><div class="text-center">Results</div>></h4>
                        </div>
                      </div>

                      <div class="modal-body custom-height-modal">
                        %{--TODO reactivate the loading --}%
                        %{--<div class="loading" id="loadingMessage" style="position:absolute; top: 50%; left: 50%">loading...</div>--}%

                        %{--<object style="width: 100%;height:800px;" data="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}" ></object>--}%
                        <object style="width: 100%;height:100%;" data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber])}" ></object>
                        %{--
                          <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px" scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0"></iframe>
                        --}%
                      </div>

                      <div class="modal-footer">
                        <button type="button" id="aux${moduleParam?.id}SaveBtn" style="display: none" class="left btn btn-success" >Save</button>
                        <button type="button" id="aux${moduleParam?.id}CloseBtn" class="btn btn-default" data-dismiss="modal">Close</button>
                      </div>
                    </div>
                  </div>
                </div>

              </g:if>
              <g:else>
                <g:if test="${p == 'timestamp'}">
                  <g:formatDate date="${bean.timestamp}" format="dd/MM/yyyy hh:mm:ss"/>
                </g:if>
                <g:else>
                    <f:display bean="${bean}"
                               property="${p}"
                               displayStyle="${displayStyle ?: 'table'}"/>
                </g:else>
              </g:else>
            </td>
          </g:else>
        </g:else>
      </g:each>
    </tr>
  </g:each>
  </tbody>
</table>
