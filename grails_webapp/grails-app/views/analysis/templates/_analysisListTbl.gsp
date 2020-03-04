<div class="row">
  <div class="col s12">
    <label id="lastUpdatedTime"></label>

    <ul id="issues-collection" class="collection">
      <g:each in="${analysisList}" var="bean" status="i">
        <li class="collection-item avatar">
          <i class="material-icons circle ${bean.analysisStatus == 1 ? 'init' : bean.analysisStatus == 2 ? 'orange' : bean.analysisStatus == 3 ? 'green' : bean.analysisStatus == -1 ? 'red' : 'done'}"></i>
          <span class="title"><strong>${bean.analysisName}</strong></span>

          <p>${bean.user.username} <br>
            <g:formatDate date="${bean.timestamp}" format="dd/MM/yyyy hh:mm:ss"/>
          </p>
          <g:if test="${bean.analysisStatus == 3}">
            <a href="#resultModal-${bean.jobNumber}" class="secondary-content tooltipped modal-trigger"
               data-tooltip="Display analysis result" data-position="left">
              <i class="material-icons">more</i>
            </a>
            <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, download: true])}"
               class="secondary-content tooltipped"
               data-tooltip="Download analysis result file" data-position="left" style="top: 52px;">
              <i class="material-icons">file_download</i>
            </a>

            <div class="modal modal-fixed-footer" id="resultModal-${bean.jobNumber}" style="width: 100% !important;height: 100% !important;">
              <div class="modal-content">
                <h4>Results</h4>

                <object style="width: 100%;height:100%;"
                        data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber])}"></object>
              </div>

              <div class="modal-footer">
                <a href="#!" class="modal-close waves-effect waves-light btn-flat">Close</a>
              </div>
            </div>
          </g:if>
          <g:elseif test="${bean.analysisStatus == -1}">
            <span class="secondary-content black-text">This analysis has failed. <br>
              There is no result file available.
            </span>
          </g:elseif>
        </li>
      </g:each>
    </ul>
  </div>
</div>

%{--<table id="analysis-table" class="highlight responsive-table mdl-data-table" style="width:100%">
  <thead>
  <tr>
    <th>Task Name</th>
    <th>Reports</th>
    <th>Task Status</th>
    <th>Submitted on</th>
    <th>Creator</th>
%{--    <th></th>--}%
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <tr id="${bean?.jobNumber}"
    <g:render template="templates/analysisListTablRow" model="[bean: bean]"/>
    </tr>
  </g:each>
  </tbody>
</table>--}%

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems, {
      endingTop: '7%'
    });

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
