<%@ page import="flowgate.Experiment; flowgate.Dataset" %>
<g:if test="${module}">
  <div class="pull-right">
    <div class="btn btn-default" onclick="$('#infoBoxModal').show();"><i class="glyphicon glyphicon-info-sign"></i>&nbsp;<g:message code="module.information.button.label" default="Pipeline Information" /></div>
  </div>
  <div class="modal fade in" id="infoBoxModal" role="dialog">
    <div class="modal-dialog" style="overflow-y: scroll; max-height:85%;  margin-top: 50px; margin-bottom:50px;">
      <div class="modal-content">
        <div class="modal-body custom-height-modal">
          <br/>
          ${raw(module?.descript ?: '<p>no module descript!</p>')}
          <br/>
          <br/>
          <div class="btn btn-default" onclick="$('#infoBoxModal').hide();"><i class="glyphicon glyphicon-remove"></i> Close</div>
          <br/>
        </div>
      </div>
    </div>
  </div>
  <br/>
  <g:each in="${module.moduleParams.sort { it.id }.findAll { it.pBasic == true }}" var="moduleParam">
    <g:if test="${moduleParam?.pType == 'ds'}">
      <div class="form-group">
        <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
        <div class="col-sm-10">
          <g:if test="${moduleParam?.pType == 'ds'}">
            <g:hiddenField name="mp-meta" value="${moduleParam?.id}" />
            <g:select class="form-control" style="width: 40%;display:inline;" id="mp-${moduleParam?.id}-ds"
                      from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                      optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" required="required" />
          </g:if>
          <p>${moduleParam?.descr}</p>
        </div>
      </div>
    </g:if>
    <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
      <div class="form-group">
        %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>--}%
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
        <div class="col-sm-10">
          <g:if test="${moduleParam?.pType == 'dir'}">
            <input class="form-control" type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                   value="${moduleParam?.defaultVal}" webkitdirectory directory multiple/>
          </g:if>
          <g:if test="${moduleParam?.pType == 'file'}">
            <input class="form-control" multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}"
                   name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
          </g:if>
          <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile !='' }">
            &nbsp;&nbsp;<b>Example:</b>&nbsp;&nbsp;<a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}" )}"><i class="glyphicon glyphicon-floppy-disk"></i> ${moduleParam?.exampleFile}</a>
          </g:if>
          <p>${moduleParam?.descr}</p>
        </div>
      </div>
    </g:if>
    <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field'}">
      <div class="form-group">
        %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>--}%
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
        <div class="col-sm-10">
          <input class="form-control" type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                 value="${moduleParam?.defaultVal}"/>
          <p>${moduleParam?.descr}</p>
        </div>
      </div>
    </g:if>
  </g:each>
  <div class="tab-pane fade in active" id="basic">
    %{--BASIC OPTIONS--}%
    %{-- Always displayed ! --}%
  </div>

  <div class="tab-pane fade" id="advanced">
  %{--ADVANCED OPTIONS--}%
    <g:each in="${module.moduleParams.sort { it.id }.findAll { it.pBasic == false }}" var="moduleParam">

      <g:if test="${moduleParam?.pType == 'ds'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pKey} [dataset]</label>--}%
          <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
          <div class="col-sm-10">
            <g:if test="${moduleParam?.pType == 'ds'}">
              <g:hiddenField name="mp-meta" value="${moduleParam?.id}" />
              <g:select style="width: 40%;display:inline" id="mp-${moduleParam?.id}-ds"
                        from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                        optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <p>${moduleParam?.descr}</p>
          </div>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>--}%
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
          <div class="col-sm-10">
            <g:if test="${moduleParam?.pType == 'dir'}">
              <input webkitdirectory directory multiple type="file" style="width: 40%;display:inline"
                     id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <g:if test="${moduleParam?.pType == 'file'}">
              <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}"
                     name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile !='' }">
              &nbsp;&nbsp;<b>Example:</b>&nbsp;&nbsp;<a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}" )}"><i class="glyphicon glyphicon-floppy-disk"></i> ${moduleParam?.exampleFile}</a>
            </g:if>
            <p>${moduleParam?.descr}</p>
          </div>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field'}">
        <div class="form-group">
          %{--<label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>--}%
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
          <div class="col-sm-10">
            <input class="form-control" type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                   value="${moduleParam?.defaultVal}"/>
            <p>${moduleParam?.descr}</p>
          </div>
        </div>
      </g:if>
    </g:each>
  </div>
</g:if>