<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="addCategoryForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <g:form controller="expFile" action="addCategory">
          <g:hiddenField name="id" value="${experiment?.id}" />
          <div class="form-group">
            <label for="mdCategory">Attribute Category *</label>
            <g:textField class="form-control" name="mdCategory" id="mdCategory" required="" />
          </div>
          <br/>
          <input class="btn btn-success" type="submit"  name="addCategory" />
          <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
        </g:form>
      </div>
    </div>
  </div>
</div>
