<tr id="job${bean.jobNumber}">
  <g:each in="${['analysisName', 'taskResultFiles', 'analysisStatus', 'timestamp', 'user.username']}" var="p" status="j">
    <g:if test="${j == 0}">
      <td>
        <g:if test="${bean?.analysisStatus != 3 && bean?.analysisStatus != 4}">
          <g:link action="edit" resource="${bean}">
            <f:display bean="${bean}" property="analysisName" displayStyle="${displayStyle ?: 'table'}"/>
          </g:link>
        </g:if>
        <g:else>
          <f:display bean="${bean}" property="analysisName" displayStyle="${displayStyle ?: 'table'}"/>
        </g:else>
      </td>
    </g:if>
    <g:else>
      <g:if test="${j == 1}">
        <td>
          <g:if test="${bean?.analysisStatus != 3 && bean?.analysisStatus != 4}">
            <g:link action="edit" resource="${bean}">
              <f:display bean="${bean}" property="analysisName" displayStyle="${displayStyle ?: 'table'}"/>
            </g:link>
          </g:if>
          <g:else>
            <g:if test="${bean?.analysisStatus == 4}">
%{--              <div class="div-as-link" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">--}%
              <div class="div-as-link" style="cursor: pointer;" onclick="openModal(${bean?.jobNumber});" >
                <i class="glyphicon glyphicon-eye-open"></i>&nbsp;<g:message code="analysis.display.report.label" default="display" />
              </div>
            </g:if>
            <g:else>
              <div class="row form-control-plaintext" >
%{--                <div class="div-as-link" style="cursor: pointer;" data-toggle="modal" data-target="#resultModal-${bean?.jobNumber}">--}%
                <div class="div-as-link" style="cursor: pointer;" onclick="openModal(${bean?.jobNumber});" >
                  <i class="glyphicon glyphicon-eye-open"></i>&nbsp;<g:message code="analysis.display.report.label" default="display" />
                </div>
                &nbsp;&nbsp;&nbsp;&nbsp;
                <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, download: true])}">
                  <i class="fa fa-floppy-o fa-lg"></i>&nbsp;<g:message code="analysis.download.report.label" default="download" />
                </a>
              </div>
            </g:else>
          </g:else>
        </td>
      </g:if>
      <g:else>
        <td <g:if test="${p == 'timestamp'}">style="vertical-align: inherit;"</g:if>>
          <g:if test="${p == 'analysisStatus'}">
            <div id="job-${bean.jobNumber}" class="form-control-plaintext">
              <i class=" fa fa-circle" style="color: ${bean.analysisStatus == 1 ? 'lightgrey' : bean.analysisStatus == 2 ? 'orange' : bean.analysisStatus == 3 ? 'lawngreen' : bean.analysisStatus == -1 ? 'red' : 'brown'}"></i>
              ${bean?.analysisStatus == 1 ? 'init' : bean?.analysisStatus == 2 ? 'pending' : bean?.analysisStatus == 3 ? 'results ready' : bean?.analysisStatus == -1 ? 'error' : 'done'}
            </div>
%{--        <g:hiddenField id="server-${bean.jobNumber}" name="server-${bean.jobNumber}" value="${bean.module.server.url}" />--}%
            <input type="hidden" id="server-${bean.jobNumber}" value="${bean.module.server.url}">
            <g:if test="${bean?.analysisStatus == 3}">
              <div id="resultModal_${bean?.jobNumber}"></div>
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
            </g:if>
          </g:if>
          <g:else>
            <g:if test="${p == 'timestamp'}">
              <g:formatDate date="${bean.timestamp}" format="dd/MM/yyyy hh:mm:ss"/>
            </g:if>
            <g:else>
              <f:display bean="${bean}" property="${p}" displayStyle="${displayStyle ?: 'table'}"/>
            </g:else>
          </g:else>
        </td>
      </g:else>
    </g:else>
  </g:each>
</tr>
%{--
<asset:javascript>
  function openModal(jobId){
     console.log("got click with id ", jobId);
<!-- var metaVal = document.getElementById("eMeta_" + mId + ".mdVal").value;-->

    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axMetaSelect')}",
      dataType: 'json',
      data: {jobId: jobId},
      success: function (data, status, xhr) {
        console.log('success');
        $("#resultModal_" + jobId.toString()).html(data.modalData);
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
         console.log('ajxComplete!');
      }
    });
  }

</asset:javascript>
--}%
