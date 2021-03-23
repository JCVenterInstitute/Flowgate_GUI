<%@ page import="flowgate.Experiment; flowgate.Dataset" %>
<g:if test="${module}">
  <div class="fixed-action-btn" style="bottom: 93px;">
    <a href="#infoBoxModal" class="btn-floating btn-large waves-effect waves-light tooltipped modal-trigger" data-tooltip="Pipeline Information" data-position="left">
      <i class="material-icons">info</i>
    </a>
  </div>

  <div class="modal modal-fixed-footer" id="infoBoxModal">
    <div class="modal-content">
      <h4>Pipeline Information</h4>

      <p>${raw(module?.descript ?: 'There is no description entered for this module!')}</p>
    </div>

    <div class="modal-footer">
      <a href="#!" class="modal-close waves-effect waves-light btn-flat">Close</a>
    </div>
  </div>
  <div id="experimentDatasetDiv"></div>
  <div id="basic">
  %{--BASIC OPTIONS--}%
  <g:each in="${module.moduleParams.sort { (it.pOrder ?: it.id) + (it.pKey ?: it.id) }.findAll { it.pBasic == true }}" var="moduleParam">
    <g:if test="${moduleParam?.pType == 'aux'}">
      <div class="col s12">
        <label for="mp-${moduleParam?.id}-ds">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="right">info</i>
          </g:if>
        </label>

        <a href="#aux${moduleParam?.id}" class="btn-floating waves-effect waves-light tooltipped modal-trigger">
          <g:message code="module.information.button.label" default="${moduleParam?.pKey}"/>
        </a>
      </div>

      <div class="modal modal-fixed-footer" id="aux${moduleParam?.id}">
        <div class="modal-content">
          <h4>${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</h4>

          <p><object data="/assets/${moduleParam?.defaultVal}"></object></p>
        </div>

        <div class="modal-footer">
          <button type="button" id="aux${moduleParam?.id}SaveBtn" style="display: none" class="left btn btn-success">Save</button>
          <a href="#!" class="modal-close waves-effect waves-light btn-flat">Close</a>
        </div>
      </div>
    </g:if>
    <g:if test="${moduleParam?.pType == 'ds'}">
      <div id="experimentDataset" class="input-field col s12">
        <g:hiddenField name="mp-meta" value="${moduleParam?.id}"/>
        <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
          <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="right">info</i>
        </g:if>
        <g:select id="mp-${moduleParam?.id}-ds"
                  from="${Dataset.findAllByExperimentAndIsActive(Experiment.get(params.eId), true)}" name="mp-${moduleParam?.id}-ds"
                  optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" required="required"/>
        <label>${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
      </div>
    </g:if>
  %{-- either dir or file --}%
    <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
      <div class="col s12">
        <div class="row">
          <div class="col s6">
            <label for="mp-${moduleParam?.id}">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
            <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
              <div class="secondary-content">
                <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="top">info</i>
              </div>
            </g:if>
            <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile != ''}">
              <a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}" class="secondary-content tooltipped"
                 data-tooltip="Download example file: ${moduleParam?.exampleFile}" data-position="top">
                <i class="material-icons">file_download</i>
              </a>
            </g:if>
            <g:if test="${moduleParam?.pType == 'dir'}">
              <div class="file-field input-field">
                <div class="btn">
                  <span>Choose File(s)</span>
                  <input type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" webkitdirectory directory multiple/>
                </div>

                <div class="file-path-wrapper">
                  <input class="file-path validate" type="text" placeholder="Upload one or more files">
                </div>
              </div>
            </g:if>
            <g:if test="${moduleParam?.pType == 'file'}">
              <div class="file-field input-field">
                <div class="btn">
                  <span>Choose File(s)</span>
                  <input multiple type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </div>

                <div class="file-path-wrapper">
                  <input class="file-path validate" type="text" placeholder="Upload one or more files">
                </div>
              </div>
            </g:if>
          </div>

          <div class="col s6">
          </div>
        </div>
      </div>
    </g:if>
  %{-- not dir, not file, not ds, not meta, not field  --}%
    <g:if
        test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field' && moduleParam?.pType != 'aux'}">
      <div class="input-field col s6">
        <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
          <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="right">info</i>
        </g:if>
        <input type="text" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}">
        <label for="mp-${moduleParam?.id}" <g:if
            test="${moduleParam?.defaultVal != null && moduleParam?.defaultVal.size() > 0}">class="active"</g:if>>${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
      </div>
    </g:if>
  </g:each>
  </div>
%{--******************************************************************************************************************************************************************************--}%
  <div id="advanced" style="display: none">
  %{--ADVANCED OPTIONS--}%
    <g:each in="${module.moduleParams.sort { it.pOrder ?: it.id }.findAll { it.pBasic == false }}" var="moduleParam">
    %{-- aux --}%
      <g:if test="${moduleParam?.pType == 'aux'}">
        <div class="input-field col s6">
          <object style="width: 100%" data="/assets/gating/index.html"></object>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'ds'}">
        <div class="input-field col s12">
          <g:hiddenField name="mp-meta" value="${moduleParam?.id}"/>
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="right">info</i>
          </g:if>
          <g:select id="mp-${moduleParam?.id}-ds"
                    from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds"
                    optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}"/>
          <label>${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
        </div>
      </g:if>
      <g:if test="${moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
        <div class="col s12">
          <div class="row">
            <div class="col s6">
              <label for="mp-${moduleParam?.id}">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
              <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
                <div class="secondary-content">
                  <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="top">info</i>
                </div>
              </g:if>
              <g:if test="${moduleParam?.exampleFile && moduleParam?.exampleFile != ''}">
                <a target="_blank" href="${resource(dir: 'files', file: "${moduleParam?.exampleFile}")}" class="secondary-content tooltipped"
                   data-tooltip="Download example file: ${moduleParam?.exampleFile}" data-position="top">
                  <i class="material-icons">file_download</i>
                </a>
              </g:if>
              <g:if test="${moduleParam?.pType == 'dir'}">
                <div class="file-field input-field">
                  <div class="btn">
                    <span>Choose File(s)</span>
                    <input type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" webkitdirectory directory multiple/>
                  </div>

                  <div class="file-path-wrapper">
                    <input class="file-path validate" type="text" placeholder="Upload one or more files">
                  </div>
                </div>
              </g:if>
              <g:if test="${moduleParam?.pType == 'file'}">
                <div class="file-field input-field">
                  <div class="btn">
                    <span>Choose File(s)</span>
                    <input multiple type="file" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                  </div>

                  <div class="file-path-wrapper">
                    <input class="file-path validate" type="text" placeholder="Upload one or more files">
                  </div>
                </div>
              </g:if>
            </div>
            <div class="col s6"></div>
          </div>
        </div>
      </g:if>
      <g:if
          test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds' && moduleParam?.pType != 'meta' && moduleParam?.pType != 'field' && moduleParam?.pType != 'aux'}">
        <div class="input-field col s6">
          <g:if test="${moduleParam?.descr != null && !moduleParam?.descr.isEmpty()}">
            <i class="material-icons prefix tooltipped" style="cursor: pointer;" data-tooltip="${moduleParam?.descr}" data-position="right">info</i>
          </g:if>
          <input type="text" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}">
          <label for="mp-${moduleParam?.id}" <g:if
              test="${moduleParam?.defaultVal != null && moduleParam?.defaultVal.size() > 0}">class="active"</g:if>>${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</label>
        </div>
      </g:if>
    </g:each>
  </div>

  <div id="gml" class="row">
    <div class="col s12">
      <g:if test="${hasDafiConfig}">
        <div class="col s12 mb-3">
          <label>
            <input name="gmlFileOption" type="radio" value="clinical" checked/>
            <span>Clinical Site</span>
          </label>
          <label>
            <input name="gmlFileOption" type="radio" value="user"/>
            <span>Custom</span>
          </label>
        </div>
        <g:if test="${gmlFiles == null}">
          <div id="clinicalSiteGml" class="col s6">
            <p>We are not able to check GML files. Please make sure clinical site is up and accessible.</p>
          </div>
        </g:if>
        <g:elseif test="${gmlFiles.size() > 0}">
          <div id="clinicalSiteGml" class="input-field col s6">
            <g:select name="gmlFile" from="${gmlFiles}" noSelection="${['': 'Select a Gml file']}"/>
            <label>GML Files</label>
          </div>
        </g:elseif>
        <g:else>
          <div id="clinicalSiteGml" class="col s6">
            <p>No GML File generated in clinical site.</p>
          </div>
        </g:else>

        <div id="userCustomGml" class="file-field input-field col s6" style="display: none">
          <div class="btn">
            <span>Select GML File</span>
            <input type="file" id="gmFileUpload" name="gmFileUpload" accept=".gml"/>
          </div>

          <div class="file-path-wrapper">
            <input class="file-path validate" type="text" placeholder="Upload a gml file">
          </div>
        </div>

        <script>
          $('input[type=radio][name="gmlFileOption"]').change(function () {
            if(this.value === "clinical") {
              $("#clinicalSiteGml").show();
              $("#userCustomGml").hide();
            } else {
              $("#userCustomGml").show();
              $("#clinicalSiteGml").hide();
            }
          });
        </script>
      </g:if>
    </div>
  </div>

  <script>
    $("#experimentDataset").appendTo("#experimentDatasetDiv");
  </script>
</g:if>
