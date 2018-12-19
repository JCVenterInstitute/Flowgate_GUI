<%@ page import="flowgate.Experiment; flowgate.Dataset" %>
<g:if test="${module}">
  <div class="pull-right">
    <button type="button" class="btn btn-default" data-toggle="modal" data-target="#infoBoxModal">
      <i class="glyphicon glyphicon-info-sign"></i>&nbsp;<g:message code="module.information.button.label" default="Pipeline Information"/>
    </button>
  </div>
  <div class="modal fade" id="infoBoxModal" role="dialog">
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <button type="button" class="close" data-dismiss="modal">&times;</button>
          <h4 class="modal-title" style="text-align: left;">Pipeline Information</h4>
        </div>

        <div class="modal-body custom-height-modal">
          ${raw(module?.descript ?: '<p>There is no module description!</p>')}
        </div>

        <div class="modal-footer">
          <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        </div>
      </div>
    </div>
  </div>
  <g:each in="${module.moduleParams.sort { (it.pOrder ?: it.id)+(it.pKey ?: it.id) }.findAll { it.pBasic == true }}" var="moduleParam">
    %{-- aux --}%
    <g:if test="${moduleParam?.pType == 'aux'}">
      <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
        <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
          <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
        </g:if>
      </label>

      <div class="col-sm-offset-2" style="padding-left: 5px;">
        <button type="button" class="btn btn-default btn-sm" data-toggle="modal" data-target="#aux${moduleParam?.id}">
          %{--<i class="glyphicon glyphicon-info-sign"></i>&nbsp;--}%<g:message code="module.information.button.label" default="${moduleParam?.pKey}"/>
        </button>
      </div>

      %{--<div class="modal fade" tabindex="-1" id="aux${moduleParam?.id}" style="width:90%;" role="dialog">--}%
      <div class="modal fade" tabindex="-1" id="aux${moduleParam?.id}" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">
        <div class="modal-dialog modal-lg">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal">&times;</button>
              <div id="myModalLabel">
                <h4 class="modal-title" style="text-align: left;">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</h4>
              </div>
            </div>

            <div class="modal-body custom-height-modal">
              %{--<object style="width: 100%;height:800px;" data="/assets/gating/index.html" ></object>--}%
              <object style="width: 100%;height:100%;" data="/assets/${moduleParam?.defaultVal}" ></object>
              %{--
                <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px"
                        scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0">
                </iframe>
              --}%
            </div>

            <div class="modal-footer">
              <button type="button" id="aux${moduleParam?.id}SaveBtn" style="display: none" class="left btn btn-success" >Save</button>
              <button type="button" id="aux${moduleParam?.id}CloseBtn" class="btn btn-default" data-dismiss="modal">Close</button>
            </div>
          </div>
        </div>
      </div>





      %{--<div class="form-group">--}%
        %{--<label class="col-sm-2"></label>--}%
        %{--<div class="col-sm-offset-2 col-sm-10">--}%
          %{--<object style="width: 100%;height:300px;" data="/assets/gating/index.html" ></object>--}%
          %{--
            <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px"
                    scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0">
            </iframe>
          --}%
        %{--</div>--}%
      %{--</div>--}%
    </g:if>
    %{-- dataset --}%
    <g:if test="${moduleParam?.pType == 'ds'}">
      <div class="form-group">
        <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
          </g:if>
        </label>

        <div class="col-sm-4">
          <g:hiddenField name="mp-meta" value="${moduleParam?.id}"/>
          <g:select class="form-control" id="mp-${moduleParam?.id}-ds"
                    from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                    optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" required="required"/>
        </div>
        <g:isOwnerOrRoles object="experiment" objectId="${params.eId}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
          <div class="col-sm-2">
            <a class="btn btn-default" href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: params.eId])}">
              <i class="fa fa-database"></i>&nbsp;Manage Datasets
            </a>
          </div>
        </g:isOwnerOrRoles>
      </div>
    </g:if>
    %{-- either dir or file --}%
    <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
      <div class="form-group">
        %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>--}%
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
          </g:if>
        </label>

        <div class="col-sm-4">
          <g:if test="${moduleParam?.pType == 'dir'}">
            <input class="form-control" type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" webkitdirectory directory multiple/>
          </g:if>
          <g:if test="${moduleParam?.pType == 'file'}">
            <input class="form-control" multiple type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
          </g:if>
        </div>
        <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile != ''}">
          <div class="col-sm-6">
            <label class="control-label">Example:</label>&nbsp;&nbsp;<a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}"><i
              class="glyphicon glyphicon-floppy-disk"></i> ${moduleParam?.exampleFile}</a>
          </div>
        </g:if>
      </div>
    </g:if>
    %{-- not dir, not file, not ds, not meta, not field  --}%
    <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field' && moduleParam?.pType != 'aux'}">
      <div class="form-group">
        %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>--}%
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
          </g:if>
        </label>

        <div class="col-sm-4">
          <input class="form-control" type="text" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
        </div>
      </div>
    </g:if>
  </g:each>
  <div class="tab-pane fade in active" id="basic">
    %{--BASIC OPTIONS--}%
    %{-- Always displayed ! --}%
  </div>
  %{--******************************************************************************************************************************************************************************--}%
  <div class="tab-pane fade" id="advanced">
  %{--ADVANCED OPTIONS--}%
    <g:each in="${module.moduleParams.sort { it.pOrder ?: it.id }.findAll { it.pBasic == false }}" var="moduleParam">
    %{-- aux --}%
      <g:if test="${moduleParam?.pType == 'aux'}">
        <div class="form-group">
          <label class="col-sm-2"></label>
          <div class="col-sm-4">
            <object style="width: 100%" data="/assets/gating/index.html" ></object>
            %{--
              <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px"
                      scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0">
              </iframe>
            --}%
          </div>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'ds'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pKey} [dataset]</label>--}%
          <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
            <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
              <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
            </g:if>
          </label>

          <div class="col-sm-4">
            <g:hiddenField name="mp-meta" value="${moduleParam?.id}"/>
            <g:select id="mp-${moduleParam?.id}-ds"
                      from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                      optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}"/>
          </div>
          <g:isOwnerOrRoles object="experiment" objectId="${params.eId}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
            <a class="btn btn-default" href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: params.eId])}">
              <i class="fa fa-database"></i>&nbsp;Manage Datasets
            </a>
          </g:isOwnerOrRoles>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>--}%
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
            <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
              <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
            </g:if>
          </label>

          <div class="col-sm-4">
            <g:if test="${moduleParam?.pType == 'dir'}">
              <input webkitdirectory directory multiple type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <g:if test="${moduleParam?.pType == 'file'}">
              <input multiple type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
          </div>
          <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile != ''}">
            <div class="col-sm-4">
              <label class="control-label">Example:</label>&nbsp;&nbsp;<a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}"><i
                class="glyphicon glyphicon-floppy-disk"></i> ${moduleParam?.exampleFile}</a>
            </div>
          </g:if>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field' && moduleParam?.pType != 'aux'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>--}%
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
            <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
              <i class="fa fa-info-circle" data-toggle="tooltip" title="${moduleParam?.descr}"></i>
            </g:if>
          </label>

          <div class="col-sm-4">
            <input class="form-control" type="text" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
          </div>
        </div>
      </g:if>
    </g:each>
  </div>

  <style>
  .tooltip-inner {
    min-width: 250px; /* the minimum width */
    max-width: 100%;
  }
  </style>
  <script type="text/javascript">
    $(document).ready(function () {
      $('[data-toggle="tooltip"]').tooltip({placement: 'right'});
    });
  </script>
</g:if>