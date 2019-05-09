<%@ page import="flowgate.ExperimentMetadataCategory; flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="addColForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
    <div class="modal-body custom-height-modal">
      <g:form controller="expFile" action="addColumn" class="form-horizontal">
        <g:hiddenField name="id" value="${experiment?.id}"/>
        <div class="form-group">
          <label for="mdCategory" class="col-sm-4 control-label">Attribute Category *</label>

          <div class="col-sm-8">
            <g:select class="form-control" id="mdCategory" name="mdCategory.id" from="${(ExperimentMetadataCategory.findAllByExperiment(experiment))?:ExperimentMetadataCategory.findOrSaveByExperimentAndMdCategory(experiment, 'Basics')}" optionKey="id"
                      optionValue="mdCategory" value="${category}" required=""/>
          </div>
        </div>

        <f:with bean="${new ExperimentMetadata()}">
          <div class="form-group">
            <label class="col-sm-4 control-label" for="mdKey">Attribute Name *</label>

            <div class="col-sm-8">
              <input type="text" class="form-control" id="mdKey" name="mdKey" placeholder="Attribute Name">
            </div>
          </div> <!-- _wrapper not used because of radio button-->
          <f:field property="dispOnFilter" label="For filtering out fcs files or creating cohorts for statistical comparison "/>
        </f:with>

        <fg:dynamicBlock itemId="eMetaValue" min="1" max="15" mdVals="[]"
                         limitReachedMsg="Sorry, you cannot specify more than 15 values"
                         removeBtnLabel="Delete">
          <g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}"/>
            <div class="col-sm-4">
              <input type="text" class="form-control" id="mdValue" name="mdValue" placeholder="Value" required>
            </div>
            <label class="col-sm-3" for="dispOrder">Disp. Order *</label>

            <div class="col-sm-2">
              <input type="text" class="form-control" id="dispOrder" name="dispOrder" placeholder="Disp. Order" required value="1">
            </div>
          </div>
        </fg:dynamicBlock>

        <div class="text-center" style="margin-top: 10px;">
          <input class="btn btn-success" type="submit" name="addCol"/>
          <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
        </div>
      </g:form>
    </div>
  </div>
</div>
