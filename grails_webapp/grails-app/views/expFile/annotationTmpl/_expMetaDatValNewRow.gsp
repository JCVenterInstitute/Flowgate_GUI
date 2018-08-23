<div id="newRow">
%{--<div id="oldRow"></div>--}%
  <f:with bean="${expMetaDatVal}" >
    <div class="row">
      <f:field property="mdValue" label="Value" />
      %{--<f:field property="mdType" label="Type" />--}%
      <g:hiddenField name="mdType" value="${expMetaDatVal.mdType ?: 'String'}" />
      &nbsp;&nbsp;<f:field property="dispOrder" label="Disp. Order" value="1"/>
      &nbsp;&nbsp;<div class="btn btn-default" title="add another value"   onclick="addValClick();" >Add Value&nbsp;<i class="fa fa-plus" ></i></div>
    </div>
  </f:with>
</div>