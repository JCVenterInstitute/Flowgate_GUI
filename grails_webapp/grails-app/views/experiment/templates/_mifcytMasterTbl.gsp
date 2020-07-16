<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
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

  </form>
</div>
<g:form controller="experiment" action="saveMiFlowData">
  <g:set var="cats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisible(experiment, true, true)*.mdCategory?.unique()}"/>
  <g:if test="${cats}">
    <g:hiddenField name="eId" value="${experiment?.id}"/>
    <g:hiddenField name="id" value="${experiment?.id}"/>

    <table id="mifcyt-annotation-table" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
      <tbody id="mifctyTbl">
      <g:each in="${cats}" var="miFlowCat">
        <tr>
          <td style="width: 20%">${miFlowCat?.mdCategory}</td>
          <g:set var="mFDats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisibleAndMdCategory(experiment, true, true, miFlowCat)?.sort { it.dispOrder }}"/>
          <td style="width: 80%">
            <g:each in="${mFDats}" var="miFlowDat">
              <g:set var="mValue" value="${ExperimentMetadataValue.findByExpMetaData(ExperimentMetadata.get(miFlowDat.id))?.mdValue}"/>
              <div class="input-field">
                <g:textField name="expMetaData-${miFlowDat.id}" value="${mValue}"/>
                <label>${miFlowDat?.mdKey}</label>
              </div>
            </g:each>
          </td>
        </tr>
      </g:each>
      </tbody>
    </table>
  </g:if>
  <g:else>
    <div>There is no MiFlowCyt data found for this experiment. Please upload an annotation file first.</div>
  </g:else>


  <div class="row">
    <div class="input-field col s12">
      <g:if test="${cats}">
        <button type="submit" class="btn waves-effect waves-light">Save</button>
      </g:if>
      <g:link class="${cats ? 'btn-flat' : 'btn'} waves-effect waves-light" controller="experiment" action="index" params="[eId: experiment?.id]">Return to experiment</g:link>
      <a class="btn-flat waves-effect waves-light modal-trigger" href="#upload-annotation-file">Upload an annotation file</a>
    </div>
  </div>
</g:form>

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
