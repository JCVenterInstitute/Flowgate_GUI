<style>
#metaBox {
  border: black 1px solid;
  padding: 10px 10px 10px 10px;
  overflow-y: scroll;
  overflow-x: hidden;
  max-height: 700px;
}
</style>
<div id="metaBox">
  <g:each in="${experiment?.expMetadatas?.findAll{it.dispOnFilter}?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">

    %{--<p>${catgy}</p>--}%
    <g:each in="${experiment?.expMetadatas?.findAll{it.mdCategory == catgy && it.dispOnFilter}?.sort{it.dispOrder} }" var="catGy">
      <div class="col-sm-3">
        <p>&nbsp;&nbsp;&nbsp;&nbsp;<strong>${catGy.mdKey}</strong></p>
        <g:each in="${experiment?.expMetadatas?.find{it.mdKey == catGy.mdKey && it.dispOnFilter}?.mdVals?.sort{it.dispOrder}}" var="mVals">
          <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<g:checkBox id="${mVals.mdValue}" class="fcs_filter" name="mVal_${mVals.id}_chb" onclick="setFilter();" checked="${mVals.mdValue in params.filters}"/>&nbsp;&nbsp;${mVals.mdValue}</p>
        </g:each>
      </div>
    </g:each>
  </g:each>
</div>
