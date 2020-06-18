<g:if test="${experiment?.expFiles}" >
<g:each in="${experiment?.expFiles?.sort { it.fileName }}" var="expFile">
  <tr>
    <td>${expFile?.fileName}</td>
    <g:each in="${experiment.expMetadatas.findAll { it.mdCategory == category }.sort { it.dispOrder }}" var="eMeta">
    <g:if test="${eMeta.visible}" >
      <td>
        <div id="tblCell_${expFile.id.toString()}_${eMeta.id.toString()}">
          <g:set var="sessMetaStr" value="meta_${eMeta.mdKey}"/>
          <g:render template="annotationTmpl/tablCellTmpl" model="[eMeta: eMeta, expFile: expFile, checked: expFile.metaDatas.find { it.mdKey == eMeta.mdKey }?.mdVal == session[sessMetaStr]]"/>
        </div>
      </td>
    </g:if>
    </g:each>
    <td>%{-- add column field --}%</td>
  </tr>
</g:each>
</g:if>