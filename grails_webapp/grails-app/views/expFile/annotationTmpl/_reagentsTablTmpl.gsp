<g:each in="${experiment?.expFiles.sort { it.fileName }}" var="expFile" status="i">
  <tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
    <td>${expFile?.fileName}</td>
  %{--<g:each in="${experiment?.expMetadatas?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">--}%
    <g:each in="${experiment.expMetadatas.find{ it.mdKey == 'Channel' }.mdVals.sort{it.dispOrder}}" var="eMeta">
      <td class="text-center">
        <div id="tblCell_${expFile.id.toString()}_${eMeta.id.toString()}">
          %{--<g:set var="sessMetaStr" value="meta_${eMeta.mdKey}"/>--}%
          %{--<g:render template="annotationTmpl/tablReagentsCellTmpl" model="[eMeta: eMeta, expFile: expFile, checked: expFile.metaDatas.find { it.mdKey == eMeta.mdKey }?.mdVal == session[sessMetaStr]]"/>--}%
          <g:render template="annotationTmpl/tablReagentsCellTmpl" model="[eMeta: eMeta, expFile: expFile]"/>
        </div>
      </td>
    </g:each>
  %{--</g:each>--}%
  %{-- add column field --}%
    <td></td>
  </tr>
</g:each>