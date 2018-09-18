<%@ page import="flowgate.ExperimentMetadataCategory; flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="editColForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <g:form controller="expFile" action="editColumn">
          <g:hiddenField name="id" value="${experiment?.id}" />
          <g:hiddenField name="metaId" value="${eMeta?.id}" />
          <g:hiddenField name="metaValId" value="${eMeta?.mdVals?.id?.join(',')}" />
          <div class="form-group">
            <label for="mdCategory">Tab *</label>
            %{--<g:select class="form-control" id="mdCategory" name="mdCategory" from="${(ExperimentMetadataCategory.findAllByExperiment(experiment).mdCategory ?: ['Default'])}" value="${category}" required="" />--}%
            <g:select class="form-control" id="mdCategory" name="mdCategory.id" from="${(ExperimentMetadataCategory.findAllByExperiment(experiment))}" optionKey="id" optionValue="mdCategory" value="${category}" required="" />
          </div>
          <f:with bean="${eMeta}" >
            <f:field property="mdKey" label="Key"/>
            <f:field property="dispOnFilter" label="Show on Filter panel" />
          </f:with>
          <fg:dynamicBlock itemId="eMetaValue" max="15" mdVals="${eMeta.mdVals*.mdValue}"
                           limitReachedMsg="Sorry, you cannot specify more than 15 values"
                           removeBtnLabel="Delete">
            <g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}" />
            <div class="form-group col-sm-6" style="padding: 0">
              <div class="col-sm-10">
                <input type="text" class="form-control" id="mdValue" name="mdValue" placeholder="Value" required>
              </div>
            </div>
            <div class="form-group col-sm-4" style="padding: 0">
              <label class="col-sm-6" for="dispOrder">Disp. Order&nbsp;*&nbsp;&nbsp;</label>
              <div class="col-sm-6">
                <input style="width: 50px" type="text" class="form-control" id="dispOrder" name="dispOrder" placeholder="Disp. Order" required value="1">
              </div>
            </div>
          </fg:dynamicBlock>
          <br/>
          <input class="btn btn-success" type="submit"  name="addCol" />
          <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
        </g:form>
      </div>
    </div>
  </div>
</div>