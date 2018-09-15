<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="addCategoryForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <g:form controller="expFile" action="addCategory">
          <g:hiddenField name="id" value="${experiment?.id}" />
          <div class="form-group">
            <label for="mdCategory">Tab *</label>
            %{--<g:select class="form-control" id="mdCategory" name="mdCategory" from="${(categories ?: ['Other']) +['Reagents']}" value="${category}" required="" />--}%

            cats = ${experiment.expMetadatas*.mdCategory.mdCategory.unique()}
            <g:textField class="form-control" name="mdCategory" id="mdCategory" required="" />
          </div>
          %{--
          <fg:dynamicBlock itemId="eMetaValue" max="15" mdVals="${eMeta.mdVals*.mdValue}"
                           limitReachedMsg="Sorry, you cannot specify more than 5 customers"
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
          --}%
          <br/>
          <input class="btn btn-success" type="submit"  name="addCategory" />
        </g:form>
      </div>
    </div>
  </div>
</div>
