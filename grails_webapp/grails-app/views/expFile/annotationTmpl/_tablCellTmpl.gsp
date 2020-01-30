<p><label>
  <g:checkBox name="eMeta_${eMeta.id}" value="${checked}" onclick="eMetaClick(${expFile.id},this.checked, ${eMeta.id});" class="filled-in"/>
  <span>${expFile.metaDatas.find { it.mdKey == eMeta.mdKey }?.mdVal}</span>
</label></p>