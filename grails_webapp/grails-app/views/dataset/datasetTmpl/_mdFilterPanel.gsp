<p>&nbsp;</p>
<p><strong>Filter</strong>
<br/>
</p>
<div id="metaBox">
  <g:each in="${experiment?.expMetadatas?.findAll{it.dispOnFilter}?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">
    <p>${catgy}</p>
    %{--<g:each in="${experiment?.expMetadatas?.findAll{(it.mdCategory == catgy) && it.dispOnFilter}?.sort{it.dispOrder} }" var="catGy">--}%
    <g:each in="${experiment?.expMetadatas?.findAll{it.mdCategory == catgy}?.sort{it.dispOrder} }" var="catGy">
      <p>&nbsp;&nbsp;&nbsp;&nbsp;<strong>${catGy.mdKey}</strong></p>
      <g:each in="${experiment?.expMetadatas?.find{it.mdKey == catGy.mdKey}?.mdVals?.sort{it.dispOrder}}" var="mVals">
        <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<g:checkBox id="mValCb_${mVals.mdValue}" name="mVal_${mVals.id}_chb" onclick="setFilter(this.id, this.checked);" checked="${mVals.mdValue in session.filters}"/>&nbsp;&nbsp;${mVals.mdValue}</p>
      </g:each>
    </g:each>
  </g:each>
</div>
