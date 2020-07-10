<%@ page import="flowgate.ExperimentMetadataCategory; flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="modal modal-fixed-footer" id="editColForm" role="dialog">
  <g:form controller="expFile" action="editColumn" class="form-horizontal">
    <div class="modal-content">
      <g:hiddenField name="id" value="${experiment?.id}"/>
      <g:hiddenField name="metaId" value="${eMeta?.id}"/>
      <g:hiddenField name="metaValId" value="${eMeta?.mdVals?.id?.join(',')}"/>
      <div class="row">
        <div class="input-field col s12">
          <g:select id="mdCategory" name="mdCategory.id" from="${(ExperimentMetadataCategory.findAllByExperiment(experiment))}" optionKey="id"
                    optionValue="mdCategory" value="${category}" required=""/>
          <label>Attribute Category *</label>
        </div>
      </div>
      <f:with bean="${eMeta}">
        <div class="row">
          <div class="input-field col s2">
            <input type="text" id="dispOrder" name="dispOrder" value="<f:display property="dispOrder"/>">
            <label class="active">Position *</label>
          </div>

          <div class="input-field col s10">
            <input type="text" id="mdKey" name="mdKey" value="<f:display property="mdKey"/>">
            <label class="active">Attribute Name *</label>
          </div>

          <div class="input-field col s12">
            <p>
              <label>
                <input type="checkbox" name="dispOnFilter" checked="<f:display property="dispOnFilter"/>"/>
                <span>For filtering out fcs files or creating cohorts for statistical comparison</span>
              </label>
            </p>
          </div>
        </div>
      </f:with>
      <fg:dynamicBlock itemId="eMetaValue" max="50" mdVals="[]"
                       limitReachedMsg="Sorry, you cannot specify more than 15 values"
                       removeBtnLabel="Delete">
        <g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}"/>
        <div class="input-field col s2">
          <input type="text" id="dispOrder" name="dispOrder" required value="1">
          <label class="active">Disp. Order *</label>
        </div>

        <div class="input-field col s8">
          <input type="text" id="mdValue" name="mdValue" required>
          <label>Attribute Value</label>
        </div>
      </fg:dynamicBlock>
    </div>

    <div class="modal-footer">
      <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
      <button class="btn-flat waves-effect waves-light" type="submit" name="addCol">Submit</button>
    </div>
  </g:form>
</div>

<script>
  //TODO Update dynamicBlock to work with material design
  const elem = '<g:hiddenField name="mdType" value="${expMetaDatVal?.mdType ?: 'String'}"/>\n' +
      '        <div class="input-field col s2">\n' +
      '          <input type="text" id="dispOrder" name="dispOrder" required value="1">\n' +
      '          <label class="active">Disp. Order *</label>\n' +
      '        </div>\n' +
      '        <div class="input-field col s8">\n' +
      '          <input type="text" id="mdValue" name="mdValue" required>\n' +
      '          <label class="active">Attribute Value</label>\n' +
      '        </div>'
  $(function () {
    const values = "${eMeta.mdVals*.mdValue}".replace('[','').replace(']','').split(', ');
    const size = ${eMeta.mdVals*.mdValue.size()};

    for (let i = 0; i < size; i++) {
      addItem("eMetaValue", elem, '1', '15', null, "Sorry, you cannot specify more than 15 values", 'Delete', "${eMeta.mdVals*.mdValue}");
    }

    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
  });
</script>
