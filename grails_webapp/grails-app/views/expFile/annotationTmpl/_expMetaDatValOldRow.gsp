<div id="oldRow"></div>
  <f:with bean="${expMetaDatVal}" >
    <div class="row">
      <f:field property="mdValue" label="Value" />
      %{--<f:field property="mdType" label="Type" />--}%
      <g:hiddenField name="mdType" value="${expMetaDatVal.mdType ?: 'String'}" />
      &nbsp;&nbsp;<f:field property="dispOrder" label="Disp. Order" value="1"/>
      &nbsp;&nbsp;<div class="btn btn-default" title="delete value" onclick="delValClick();" >Del Value&nbsp;<i class="fa fa-trash" ></i></div>
    </div>
  </f:with>
%{--<div id="newRow"></div>--}%