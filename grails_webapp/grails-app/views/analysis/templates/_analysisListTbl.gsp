<%@ page import="flowgate.UtilsService; flowgate.AnalysisController" %>
<div class="row">
  <div class="col s12">
    <label id="lastUpdatedTime"></label>

    <ul id="issues-collection" class="collection collapsible">
      <g:each in="${analysisList}" var="bean" status="i">
        <li class="collection-item collapsible-item avatar" <g:if test="${bean.analysisStatus == 3 && bean.module.server.isGenePatternServer()}">style="min-height: 125px;"</g:if>>
          <g:if test="${bean.jobNumber.contains(',')}">
            <div class="collapsible-header collection-header">
          </g:if>
          <i class="material-icons circle ${bean.analysisStatus == 1 ? 'init' : bean.analysisStatus == 2 ? 'orange' : bean.analysisStatus == 3 || bean.analysisStatus == 4 ? 'green' : bean.analysisStatus == -1 ? 'red' : 'done'}"></i>
          <span class="title"><strong>Analysis Name:</strong> ${bean.analysisName}</span>

          <p><strong>Server: </strong>${bean.module.server.name}<br>
            <strong>Pipeline: </strong>${bean.module.label != null ? bean.module.label : bean.module.title != null ? bean.module.title : bean.module.name}
          </p>

          <div class="middle-content">
            <p><strong>Submitted by: </strong>${bean.user.username}<br>
              <strong>Submission date: </strong> <g:formatDate date="${bean.timestamp}" format="dd/MM/yyyy hh:mm:ss"/>
              <g:if test="${bean.dateCompleted}">
                <br><strong>Completion date: </strong><g:formatDate date="${bean.dateCompleted}" format="dd/MM/yyyy hh:mm:ss"/>
              </g:if>
            </p>
          </div>
          <g:if test="${bean.jobNumber.contains(',')}">
            <div class="secondary-content tooltipped" data-tooltip="Expand to see details" data-position="left" style="top: 10px;">
              <i class="material-icons">expand_more</i>
            </div>
          </g:if>
          <g:elseif test="${bean.analysisStatus == 3 || bean.analysisStatus == 4}">
            <g:if test="${bean.analysisStatus == 3}">
              <g:if test="${bean.module.server.isGenePatternServer()}">
                <div class="secondary-content-with-links">
                  <a href="#resultModal-${bean.jobNumber}" class="modal-trigger">
                    <i class="material-icons">more</i> Display Analysis Result
                  </a><br>
                  <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, download: true])}">
                    <i class="material-icons">file_download</i> Report File
                  </a><br>
                  <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, config: 'events', download: true])}">
                    <i class="material-icons">file_download</i> Events File
                  </a><br>
                  <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, config: 'percentages', download: true])}">
                    <i class="material-icons">file_download</i> Percentages File
                  </a>
                </div>
              </g:if>
              <g:else>
                <a href="#resultModal-${bean.jobNumber}" class="secondary-content tooltipped modal-trigger"
                   data-tooltip="Display analysis result" data-position="left" style="top: 13px;">
                  <i class="material-icons">more</i>
                </a>
                <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber, download: true])}"
                   class="secondary-content tooltipped"
                   data-tooltip="Download analysis result file" data-position="left" style="top: 52px;">
                  <i class="material-icons">file_download</i>
                </a>
              </g:else>
            </g:if>
            <g:elseif test="${bean.analysisStatus == 4}">
              <span class="secondary-content black-text">This analysis has been completed but there is no report file detected. Please check error file for the details.</span>
              <a href="#resultModal-${bean.jobNumber}" class="secondary-content tooltipped modal-trigger" style="top: 52px;"
                 data-tooltip="Display error file" data-position="left">
                <i class="material-icons">more</i>
              </a>
            </g:elseif>

            <div class="modal modal-fixed-footer" id="resultModal-${bean.jobNumber}" style="width: 100% !important;height: 100% !important;">
              <div class="modal-content">
                <g:if test="${bean.analysisStatus == 3}">
                  <h4>Results</h4>

                  <object style="width: 100%;height:100%;"
                          data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: bean?.jobNumber])}"></object>
                </g:if>
                <g:elseif test="${bean.analysisStatus == 4}">
                  <h4>Error Details</h4>

                  <object style="width: 100%;height:100%;"
                          data="${g.createLink(controller: 'analysis', action: 'downloadErrorFile', params: [analysisId: bean?.id, jobNr: bean?.jobNumber])}"></object>
                </g:elseif>
              </div>

              <div class="modal-footer">
                <a href="#!" class="modal-close waves-effect waves-light btn-flat">Close</a>
              </div>
            </div>
          </g:elseif>
          <g:elseif test="${bean.analysisStatus == -1}">
            <span class="secondary-content black-text" style="top: 10px;">This analysis has failed. <br>
              There is no result file available.
            </span>
          </g:elseif>

          <g:if test="${bean.jobNumber.contains(',')}">
            </div>
            <div class="collapsible-body collection-body">
              <g:if test="${bean.analysisStatus == 2}">
                <p>We are waiting all submissions to be completed for this analysis.</p>
              </g:if>
              <g:elseif test="${bean.analysisStatus == 3 || bean.analysisStatus == 4}">
                <ul class="collection">
                  <%
                    String[] jobIds = bean.jobNumber.split(',');
                    for (String jobId : jobIds) {
                      String fcsName = flowgate.UtilsService.receiveInputFileNameForJob(bean, jobId);
                  %>
                  <li class="collection-item">
                    <span class="title"><strong>${fcsName}</strong></span>
                    <g:if test="${bean.analysisStatus == 3}">
                      <a href="#resultModal-${jobId}" class="secondary-content tooltipped modal-trigger"
                         data-tooltip="Display analysis result" data-position="left" style="position: sticky;">
                        <i class="material-icons">more</i>
                      </a>
                      <a href="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: jobId, download: true])}"
                         class="secondary-content tooltipped"
                         data-tooltip="Download analysis result file" data-position="left" style="position: sticky; right: 52px;">
                        <i class="material-icons">file_download</i>
                      </a>
                    </g:if>
                    <g:elseif test="${bean.analysisStatus == 4}">
                      <span
                          class="secondary-content black-text">This analysis has been completed but there is no report file detected. Please check error file for the details.</span>
                      <a href="#resultModal-${jobId}" class="secondary-content tooltipped modal-trigger" style="position: sticky; right: 52px;"
                         data-tooltip="Display error file" data-position="left">
                        <i class="material-icons">more</i>
                      </a>
                    </g:elseif>

                    <div class="modal modal-fixed-footer" id="resultModal-${jobId}" style="width: 100% !important;height: 100% !important;">
                      <div class="modal-content">
                        <g:if test="${bean.analysisStatus == 3}">
                          <h4>Results</h4>

                          <object style="width: 100%;height:100%;"
                                  data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: bean?.id, jobNr: jobId])}"></object>
                        </g:if>
                        <g:elseif test="${bean.analysisStatus == 4}">
                          <h4>Error Details</h4>

                          <object style="width: 100%;height:100%;"
                                  data="${g.createLink(controller: 'analysis', action: 'downloadErrorFile', params: [analysisId: bean?.id, jobNr: jobId])}"></object>
                        </g:elseif>
                      </div>

                      <div class="modal-footer">
                        <a href="#!" class="modal-close waves-effect waves-light btn-flat">Close</a>
                      </div>
                    </div>
                  </li>
                  <%
                    }
                  %>
                </ul>
              </g:elseif>
            </div>
          </g:if>
        </li>
      </g:each>
    </ul>
  </div>

  <g:if test="${analysisCount > 10}">
    <ul class="pagination" id="pagination">
      <g:paginate params="${[eId: params.eId]}" total="${analysisCount ?: 0}" prev="chevron_left" next="chevron_right"/>
    </ul>
  </g:if>
</div>

<style>
@media only screen and (min-width: 993px) {
  .collection-item .middle-content {
    position: absolute;
    top: 10px;
    left: 50%;
  }
}

.secondary-content-with-links {
  position: absolute;
  top: 16px;
  right: 16px;
}

.secondary-content-with-links a {
  color: #26a69a;
  padding: 2px 5px;
}

.secondary-content-with-links > a > i {
  vertical-align: bottom;
}

.secondary-content-with-links a:hover {
  box-shadow: none;
  background-color: #f1f3f4;
}
</style>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems, {
      endingTop: '7%'
    });

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);

    var collapsibleElems = document.querySelectorAll('.collapsible');
    M.Collapsible.init(collapsibleElems);

    updatePaginationToMaterial($('#pagination'));
  });
</script>
