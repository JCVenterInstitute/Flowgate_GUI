<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal" id="editCategoryForm">
  <g:form controller="expFile" action="updateCategory">
    <div class="modal-content">
      <g:hiddenField name="id" value="${category?.id}"/>
      <h4>Edit category</h4>

      <div class="row">
        <div class="input-field col s12">
          <g:textField class="form-control" name="mdCategory" id="mdCategory" required="" value="${category?.mdCategory}"/>
          <label class="active">Attribute Category *</label>
        </div>
      </div>
    </div>

    <div class="modal-footer">
      <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
      <g:actionSubmit controller="expFile" action="deleteCategory" value="Delete" onclick="return confirm('Are you sure???')" id="${category.id}"
                      class="waves-effect waves-light btn-flat"/>
      <button type="submit" class="waves-effect waves-light btn-flat">Submit</button>
    </div>
  </g:form>
</div>
