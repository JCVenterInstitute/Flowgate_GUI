<table id="analysis-table" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <th>Task Name</th>
    <th>Task Status</th>
    <th>Submitted on</th>
    <th>Creator</th>
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <tr>
      <g:each in="${['analysisName', 'analysisStatus', 'timestamp', 'user.username']}" var="p" status="j">
        <g:if test="${j == 0}">
          <td>
          %{-- TODO when results ready redirect to show results page --}%
            <g:if test="${bean?.analysisStatus != 3 && bean?.analysisStatus != 4}">
              <g:link action="edit" resource="${bean}">
                <f:display bean="${bean}"
                           property="${p}"
                           displayStyle="${displayStyle ?: 'table'}"/>%{-- &nbsp;Job=${bean.jobNumber} --}%
              </g:link>
            </g:if>
            <g:else>
              <g:if test="${bean?.analysisStatus == 4}">
                <div class="div-as-link" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">
                  <f:display bean="${bean}"
                             property="${p}"
                             displayStyle="${displayStyle ?: 'table'}"/>%{--&nbsp;Job=${bean.jobNumber}--}%
                </div>
              </g:if>
              <g:else>
                <g:link action="showResults" resource="${bean}">
                  <f:display bean="${bean}"
                             property="${p}"
                             displayStyle="${displayStyle ?: 'table'}"/>%{-- &nbsp;Job=${bean.jobNumber} --}%
                </g:link>
              </g:else>
            </g:else>
          </td>
        </g:if>
        <g:else>
          <td>
            <g:if test="${p == 'analysisStatus'}">
              <div id="job-${bean.jobNumber}" class="form-control-plaintext">
                <i class=" fa fa-circle"
                   style="color: ${bean.analysisStatus == 1 ? 'lightgrey' : bean.analysisStatus == 2 ? 'orange' : bean.analysisStatus == 3 ? 'lawngreen' : bean.analysisStatus == -1 ? 'red' : 'brown'}"></i>
                &nbsp;&nbsp;&nbsp;
                ${bean.analysisStatus == 1 ? 'init' : bean.analysisStatus == 2 ? 'pending' : bean.analysisStatus == 3 ? 'results ready' : bean.analysisStatus == -1 ? 'error' : 'done'}
              </div>
            </g:if>
            <g:else>
              <g:if test="${p == 'timestamp'}">
                <div class="form-control-plaintext"><g:formatDate date="${bean.timestamp}" format="dd/MM/yyyy hh:mm:ss"/></div>
              </g:if>
              <g:else>
                  <f:display bean="${bean}"
                             property="${p}"
                             displayStyle="${displayStyle ?: 'table'}"/>
              </g:else>
            </g:else>
          </td>
        </g:else>
      </g:each>
    </tr>
  </g:each>
  </tbody>
</table>
