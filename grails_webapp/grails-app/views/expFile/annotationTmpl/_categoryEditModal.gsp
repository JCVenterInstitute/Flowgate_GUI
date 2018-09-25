<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="editCategoryForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <g:form controller="expFile" action="updateCategory">
          %{--<g:hiddenField name="id" value="${experiment?.id}" />--}%
          <g:hiddenField name="id" value="${category?.id}" />
          cat=${category}
          <div class="form-group">
            <label for="mdCategory">Attribute Category *</label>
            <g:textField class="form-control" name="mdCategory" id="mdCategory" required="" value="${category?.mdCategory}"/>
          </div>
          <br/>
          <input class="btn btn-success" type="submit"  name="addCategory" />
          <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
          <g:actionSubmit controller="expFile" action="deleteCategory" value="Delete" onclick="return confirm('Are you sure???')" id="${category.id}"/>
        </g:form>
      </div>
    </div>
  </div>
</div>
