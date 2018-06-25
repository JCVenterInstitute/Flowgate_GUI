<table class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <g:sortableColumn class="text-center" property="analysisName" title="${message(code: 'propTitle.analysisName.column', default: 'Task Name')}"/>
    <g:sortableColumn class="text-center" property="analysisStatus" title="${message(code: 'propTitle.analysisStatus.column', default: 'Task Status')}"/>
    <g:sortableColumn class="text-center" property="timestamp" title="${message(code: 'propTitle.timestamp.column', default: 'Submitted on')}"/>
    <g:sortableColumn class="text-center" property="user" title="${message(code: 'propTitle.User.column', default: 'Creator')}"/>
    %{--<th class="text-center sortable">Action</th>--}%
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
      <g:each in="${['analysisName', 'analysisStatus', 'timestamp', 'user.username', 'delete']}" var="p" status="j">
        <g:if test="${j == 4}">
          %{--<td class="text-center">
            <g:link style="color: black" controller="analysis" action="del" id="${bean.id}"><i class="glyphicon glyphicon-trash"></i></g:link>
          </td>--}%
        </g:if>
        <g:else>
          <g:if test="${j == 0}">
            <td class="text-center">
            %{--<sec:ifAnyGranted roles="ROLE_Administrator">--}%
            %{--<g:link method="GET" resource="${bean}">--}%

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
            %{--</sec:ifAnyGranted>--}%
            %{--<sec:ifAnyGranted roles="ROLE_Administrator">--}%
            %{--  Show User method  --}%
            %{--
            <g:link method="GET" resource="${bean}">
                <f:display bean="${bean}"
                           property="${p}"
                           displayStyle="${displayStyle?:'table'}" />
            </g:link>
            --}%
            %{--  </sec:ifAnyGranted> --}%
            %{--<sec:ifNotGranted roles="ROLE_Administrator">
                <f:display bean="${bean}"
                           property="${p}"
                           displayStyle="${displayStyle?:'table'}" />
            </sec:ifNotGranted>--}%
            </td>
          </g:if>
          <g:else>
            <td class="text-center">
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
                  <div style="margin-left: 40%;">
                    <f:display bean="${bean}"
                               property="${p}"
                               displayStyle="${displayStyle ?: 'table'}"/>
                  </div>
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

<g:if test="${analysisCount > 10}">
  <div class="pagination">
    <g:paginate total="${analysisCount ?: 0}"/>
  </div>
</g:if>
