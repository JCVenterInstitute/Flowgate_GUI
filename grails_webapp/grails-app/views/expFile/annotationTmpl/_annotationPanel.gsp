<p>&nbsp;</p>
%{--<p>anSels=${session.anSels}</p>--}%
<div id="annotationPanel">
    <table class="table table-hover">
    <g:each in="${experiment?.expMetadatas?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">
        <g:each in="${experiment.expMetadatas.findAll{it.mdCategory== catgy}.sort{it.dispOrder} }" var="eMeta">
            <tr>
                %{--<td><g:checkBox name="eMeta_${eMeta.id}_chb" /></td>--}%
                <td><g:checkBox name="eMeta_${eMeta.id}" checked="${eMeta.id in session.anSels}" /></td>
                <td>${eMeta.mdKey}</td>
                <td class="text-center">
                    <g:if test="${eMeta.mdVals.size()>1}" >
                        <g:select name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort{it.dispOrder}}" optionValue="mdValue" value="" />
                    </g:if>
                    <g:else>
                        <g:hiddenField name="eMeta_${eMeta.id}.mdVal" value="${eMeta.mdVals.mdValue.join(',')}" />
                        ${eMeta.mdVals.mdValue.join(',')}
                    </g:else>
                </td>
                %{--<td><div class="btn btn-default"><i class="fa fa-trash" ></i></div></td>--}%
                <td><i class="fa fa-trash" style="cursor: pointer" onclick="alert('Delete eMeta.id=${eMeta.id.toString()} not implemented yet!');" ></i></td>
            </tr>
        </g:each>
    </g:each>
    </table>
</div>