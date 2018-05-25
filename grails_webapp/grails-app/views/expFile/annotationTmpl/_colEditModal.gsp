<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
Modal loaded!
<div class="modal fade in" id="editColForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        Edit
        <g:form controller="expFile" action="editColumn">
          <g:hiddenField name="id" value="${experiment.id}" />
          <g:hiddenField name="metaId" value="${eMeta.id}" />
          <g:hiddenField name="metaValId" value="${eMeta?.mdVals.id.join(',')}" />

          <div class="form-group">
            <label for="mdCategory">Tab *</label>
            %{--<input type="text" class="form-control" id="${property}" name="${property}" placeholder="${label}" ${required}>--}%
            <g:select class="form-control" id="mdCategory" name="mdCategory" from="['Demographics','Visit','Stimulation','Reagents']" value="${category}" required="" />
          </div>
          <f:with bean="${new ExperimentMetadata()}" >
          %{--<f:with bean="${experimentMetaData}" >--}%
            <f:field property="mdKey" label="Key"/>
          </f:with>
        %{--<f:with bean="${experimentMetaDataValue}" >--}%
          <f:with bean="${new ExperimentMetadataValue()}" >
            <f:field property="mdValue" label="Value" />
            <f:field property="mdType" label="Type" />
            <f:field property="dispOrder" label="Disp. Order" value="1"/>
          </f:with>
          <i title="add value" class="fa fa-plus" ></i>
          <br/>
          <input class="btn btn-success" type="submit"  name="addCol" />
        </g:form>
      </div>
    </div>
  </div>
</div>
<script>
  $(document).ready(function(){
     // alert('should open modal now');
      $("#colEditModal").modal({
        show: 'true'
      });
  });
</script>