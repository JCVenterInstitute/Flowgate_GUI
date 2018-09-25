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
            <label for="mdCategory">Attribute Category *</label>
            <g:select class="form-control" id="mdCategory" name="mdCategory.id" from="${(ExperimentMetadataCategory.findAllByExperiment(experiment))}" optionKey="id" optionValue="mdCategory" value="${category}" required="" />
          </div>
          <f:with bean="${eMeta}" >
            <f:field property="mdKey" label="Attribute Name"/>
            <g:if env="development">
              <f:field property="dispOrder" label="Position" />
            </g:if>
            <f:field property="dispOnFilter" label="For filtering out fcs files or creating cohorts for statistical comparison " />
          </f:with>
          <fg:dynamicBlock itemId="eMetaValue" max="15" mdVals="${eMeta.mdVals*.mdValue}"
                           limitReachedMsg="Sorry, you cannot specify more than 15 values"
                           removeBtnLabel="Delete">
            <g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}" />
            <g:if env="development">
            %{--<g:if env="production">--}%
              <div class="form-group col-sm-6" style="padding: 0">
                <div class="col-sm-10">
            </g:if>
            <g:else>
              <div class="form-group col-sm-10" style="padding: 0">
                <div class="col-sm-10">
            </g:else>
                <input type="text" class="form-control" id="mdValue" name="mdValue" placeholder="Value" required>
              </div>
            </div>
            <g:if env="development">
            %{--<g:if env="production">--}%
              <div class="form-group col-sm-4" style="padding: 0">
              <label class="col-sm-6" for="dispOrder">Disp. Order&nbsp;*&nbsp;&nbsp;</label>
              <div class="col-sm-6">
                <input style="width: 50px" type="text" class="form-control" id="dispOrder" name="dispOrder" placeholder="Disp. Order" required value="1">
              </div>
            </div>
            </g:if>
            <g:else>
              <g:hiddenField name="dispOrder" value="1" />
            </g:else>
          </fg:dynamicBlock>
          <br/>
          <br/>
          <div class="text-center">
            <input class="btn btn-success" type="submit"  name="addCol" />
            <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
          </div>
        </g:form>
      </div>
    </div>
  </div>
</div>