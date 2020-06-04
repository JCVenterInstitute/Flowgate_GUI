<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<g:set var="cats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisible(experiment, true, true)*.mdCategory?.unique()}" />
<g:if test="${cats}" >
  <g:form controller="experiment" action="saveMiFlowData" >
    <g:hiddenField name="eId" value="${experiment?.id}" />
    <g:hiddenField name="id" value="${experiment?.id}" />
    <div class="pull-right">
      <g:actionSubmit class="btn btn-success" value="Save and Return" action="saveMiFlowData" />
    </div>
    <br/>
    <br/>
    <br/>
  <div class="scroll-wrapper" style="overflow-x: scroll; overflow-y:hidden;">
    <div class="scroll-top" style="height: 1px;"></div>
    <div id="wholeTbl" style="overflow-y: auto;margin-bottom: 100px;">
      <table id="mifcyt-annotation-table" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%" >
        <tbody id="mifctyTbl">
          <g:each in="${cats}" var="miFlowCat" >
          <tr>
            <td style="width: 20%">${miFlowCat?.mdCategory}</td>
            <g:set var="mFDats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisibleAndMdCategory(experiment, true, true, miFlowCat)?.sort {it.dispOrder}}"/>
            <td style="width: 80%">
            <g:each in="${mFDats}" var="miFlowDat" >
              <g:set var="mValue" value="${ExperimentMetadataValue.findByExpMetaData(ExperimentMetadata.get(miFlowDat.id))?.mdValue}"/>
              <p><span class="col-md-3">${miFlowDat?.mdKey}</span>&nbsp;<g:textField style="width: 70%" name="expMetaData-${miFlowDat.id}" value="${mValue}" /></p>
            </g:each>
            </td>
          </tr>
          </g:each>
        </tbody>
      </table>
    </div>
  </div>
  </g:form>
</g:if>
<g:else>
  <div class="text-center alert alert-warning">Missing MiFlowCyt data!</div>
  <div class="row">
    <div class="input-field col s12">
      <g:link class="btn waves-effect waves-light" controller="experiment" action="index" params="[eId: experiment?.id]">Return to experiment</g:link>
      <a class="btn-flat waves-effect waves-light modal-trigger" href="#upload-annotation-file">Upload an MIFlowCyt File</a>
    </div>
  </div>

  <div id="upload-annotation-file" class="modal modal-fixed-footer">
    <form id="upldForm" action="${g.createLink(controller: 'experiment', action: 'importMifcyt', id: experiment?.id)}" method="post" enctype="multipart/form-data">
      <div class="modal-content">
        <h4>Upload an MIFlowCyt file</h4>

        <g:radioGroup name="separator"
                      labels="['CSV (Comma)', 'TSV (Tab)']"
                      values="[',', '\t']"
                      value=",">
          <p>
            <label>
              ${it.radio}
              <span>${it.label}</span>
            </label>
          </p>
        </g:radioGroup>

        <div class="file-field input-field">
          <div class="btn">
            <span>Select MIFlowCyt File</span>
            <input type="file" name="mifcytFile" accept=".csv,.tsv" required="required">
          </div>

          <div class="file-path-wrapper">
            <input class="file-path validate" type="text" placeholder="Select a CSV or TSV file">
          </div>
        </div>
      </div>

      <div class="modal-footer">
        <button id="submitBtn" class="btn-flat waves-effect waves-light" type="submit" onclick="submitClick();">Upload</button>
        <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
      </div>

      <script>
        document.addEventListener('DOMContentLoaded', function () {

          var modalElems = document.querySelectorAll('.modal');
          M.Modal.init(modalElems);
        });

        function submitClick() {
          $("#submitBtn").attr("disabled", true);
          $("#upldForm").submit();
          $("#clickMsg").html('<p class="alert-info">Uploading MIFlowCyt file ... </p>');
        }
      </script>
    </form>
  </div>
</g:else>
