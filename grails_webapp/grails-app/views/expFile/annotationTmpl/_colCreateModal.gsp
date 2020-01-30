<%@ page import="flowgate.ExperimentMetadataCategory; flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div id="add-new-attribute" class="modal modal-fixed-footer">
  <g:form controller="expFile" action="addColumn">
    <div class="modal-content">
      <g:hiddenField name="id" value="${experiment?.id}"/>
      <h4>Add a new attribute</h4>

      <div class="input-field col s12">
        <g:select id="mdCategory" name="mdCategory.id"
                  from="${(ExperimentMetadataCategory.findAllByExperiment(experiment)) ?: ExperimentMetadataCategory.findOrSaveByExperimentAndMdCategory(experiment, 'Basics')}"
                  optionKey="id"
                  optionValue="mdCategory" value="${category}" required=""/>
        <label>Attribute Category *</label>
      </div>

      <f:with bean="${new ExperimentMetadata()}">
        <div class="input-field col s12">
          <input type="text" name="mdKey">
          <label for="mdKey">Attribute Name *</label>
        </div>

        <div class="col s12">
          <label>
            <input type="hidden" name="_dispOnFilter">
            <input type="checkbox" class="filled-in" checked="checked" name="dispOnFilter"/>
            <span>For filtering out fcs files or creating cohorts for statistical comparison</span>
          </label>
        </div>
      </f:with>

      <fg:dynamicBlock itemId="eMetaValue" min="1" max="15" mdVals="[]"
                       limitReachedMsg="Sorry, you cannot specify more than 15 values"
                       removeBtnLabel="Delete">
        <g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}"/>
        <div class="input-field col s2">
          <input type="text" id="dispOrder" name="dispOrder" required value="1">
          <label>Disp. Order *</label>
        </div>

        <div class="input-field col s10">
          <input type="text" id="mdValue" name="mdValue" required>
          <label>Attribute Value</label>
        </div>
      </fg:dynamicBlock>
    </div>

    <div class="modal-footer">
      <button type="submit" class="waves-effect waves-light btn-flat">Submit</button>
      <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    </div>
  </g:form>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
  });
</script>
