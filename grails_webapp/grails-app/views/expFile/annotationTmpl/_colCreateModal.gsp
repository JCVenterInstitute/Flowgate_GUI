<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal fade in" id="addColForm" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <g:form controller="expFile" action="addColumn">
          <g:hiddenField name="id" value="${experiment.id}" />
          <div class="form-group">
            <label for="mdCategory">Tab *</label>
            %{--<input type="text" class="form-control" id="${property}" name="${property}" placeholder="${label}" ${required}>--}%
            <g:select class="form-control" id="mdCategory" name="mdCategory" from="['Demographics','Visit','Stimulation','Reagents']" value="Demographics" required="" />
          </div>

          <f:with bean="${new ExperimentMetadata()}" >
          %{--<f:with bean="${experimentMetaData}" >--}%
            <f:field property="mdKey" label="Key"/>
          </f:with>
          <br/>
          <g:each in="${(expMetaDatValList && expMetaDatValList.size()>0) ? expMetaDatValList : new ExperimentMetadataValue()}" var="expMetaDatVal" >
          %{-- <g:set var="expMetaDatVal" value="${expMetaDatValList ? new ExperimentMetadataValue()}" /> --}%
          <f:with bean="${expMetaDatVal}" >
            <div class="row">
              <f:field property="mdValue" label="Value" />
              %{--<f:field property="mdType" label="Type" />--}%
              <g:hiddenField name="mdType" value="${expMetaDatVal.mdType}" />
              &nbsp;&nbsp;<f:field property="dispOrder" label="Disp. Order" value="1"/>
              &nbsp;&nbsp;<div class="btn btn-default" title="add another value"  >Add Value <i class="fa fa-plus" ></i></div>
            </div>
          </f:with>
          <br/>
          </g:each >
          <input class="btn btn-success" type="submit"  name="addCol" />
        </g:form>
      </div>
    </div>
  </div>
</div>