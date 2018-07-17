<%@ page import="flowgate.Experiment; flowgate.Dataset" %>
<g:if test="${module}">
  <g:each in="${module.moduleParams.sort { it.id }.findAll { it.pBasic == true }}" var="moduleParam">
    <g:if test="${moduleParam?.pType == 'ds'}">
      <div class="form-group">
        <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pKey} [dataset]</label>
        <div class="col-sm-10">
        <g:if test="${moduleParam?.pType == 'ds'}">
          <g:hiddenField name="mp-meta" value="${moduleParam?.id}" />
          <g:select class="form-control" style="width: 40%;display:inline;" id="mp-${moduleParam?.id}-ds"
                    from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                    optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" required="required" />
        </g:if>
        </div>
      </div>
    </g:if>
    <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
      <div class="form-group">
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>
        <div class="col-sm-10">
          <g:if test="${moduleParam?.pType == 'dir'}">
            <input class="form-control" type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                   value="${moduleParam?.defaultVal}" webkitdirectory directory multiple/>
          </g:if>
          <g:if test="${moduleParam?.pType == 'file'}">
            <input class="form-control" multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}"
                   name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
          </g:if>
        </div>
      </div>
    </g:if>
    <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds'}">
      <div class="form-group">
        <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>
        <div class="col-sm-10">
          <input class="form-control" type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                 value="${moduleParam?.defaultVal}"/>
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
          <label for="mp-${moduleParam?.id}-ds" class="col-sm-2 control-label">${moduleParam?.pKey} [dataset]</label>
          <div class="col-sm-10">
            <g:if test="${moduleParam?.pType == 'ds'}">
              <g:hiddenField name="mp-meta" value="${moduleParam?.id}" />
              <g:select style="width: 40%;display:inline" id="mp-${moduleParam?.id}-ds"
                        from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                        optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}"/>
            </g:if>
          </div>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
        <div class="form-group">
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey} [dir/file]</label>
          <div class="col-sm-10">
            <g:if test="${moduleParam?.pType == 'dir'}">
              <input webkitdirectory directory multiple type="file" style="width: 40%;display:inline"
                     id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <g:if test="${moduleParam?.pType == 'file'}">
              <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}"
                     name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
          </div>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds'}">
        <div class="form-group">
          <label for="mp-${moduleParam?.id}" class="col-sm-2 control-label">${moduleParam?.pKey}</label>
          <div class="col-sm-10">
            <input class="form-control" type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}"
                   value="${moduleParam?.defaultVal}"/>
          </div>
        </div>
      </g:if>
    </g:each>
  </div>
</g:if>