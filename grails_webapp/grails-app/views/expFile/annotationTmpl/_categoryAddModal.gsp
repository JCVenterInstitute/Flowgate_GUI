<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div id="add-new-category" class="modal">
  <g:form controller="expFile" action="addCategory">
    <div class="modal-content">
      <g:hiddenField name="id" value="${experiment?.id}"/>
      <h4>Add a new annotation category</h4>

      <div class="input-field">
        <g:textField name="mdCategory" id="mdCategory" required=""/>
        <label for="mdCategory">Attribute Category</label>
      </div>
    </div>

    <div class="modal-footer">
      <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
      <button type="submit" class="waves-effect waves-light btn-flat">Submit</button>
    </div>
  </g:form>
</div>
