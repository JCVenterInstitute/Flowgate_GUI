%{--<g:checkBox name="eMeta_${eMeta.id}" value="${checked}" onclick="eMetaClick(${expFile.id},this.checked, ${eMeta.id});" />--}%
${expFile.reagentPanel.reagents.find{it.channel == eMeta.mdValue}.fluorochrome} - ${expFile.reagentPanel.reagents.find{it.channel == eMeta.mdValue}.marker}