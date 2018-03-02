<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div class="row" style="max-width: none; padding: 0" >
    <div class="col-sm-offset-1 col-sm-10">
        %{--<div class="fixed-table-container">--}%
            %{--<div class="header-background"> </div>--}%
                %{--<div class="fixed-table-container-inner extrawrap">--}%
        <table cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
            <thead>
                <tr>
                    <g:sortableColumn property="expFile" title="${message(code: 'expFile.label', default: 'Exp. File')}" >
                       <p><br/></p>
                    </g:sortableColumn>
                    <g:each in="${experiment?.expMetadatas?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">
                        <g:each in="${experiment.expMetadatas.findAll{it.mdCategory== catgy}.sort{it.dispOrder} }" var="eMeta">
                            <th class="sortable">
                                <p class="text-center" ><a style="padding-right: 0" class="noLinkBlack" href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment.id, params:[sort:p])}"><i class="fa fa-sort" ></i></a></p>
                                <p class="text-center" ><i class="fa fa-eye"></i>&nbsp;<g:checkBox name="eMetaShow_${eMeta.id}" checked="${false}"/></p>
                                <p class="text-center" ><i class="fa fa-filter"></i>&nbsp;<g:checkBox name="eMetaFilter_${eMeta.id}" checked="${false}"/></p>
                                <p>&nbsp;</p>
                                <p class="text-center" >${eMeta.mdKey}</p>
                                <p class="text-center" >
                                    <g:if test="${eMeta.mdVals.size()>1}" >
                                        <g:select id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort{it.dispOrder}}" optionKey="id" optionValue="mdValue" value="" onchange="eMetaValueChange(${eMeta.id}, this.value);"/>
                                    </g:if>
                                    <g:else>
                                        <g:hiddenField id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" value="${eMeta?.mdVals.id.join(',')}" />
                                        ${eMeta.mdVals.mdValue.join(',')}
                                    </g:else>
                                    &nbsp;<i style="font-size: xx-small;cursor: pointer" class="fa fa-pencil" onclick="alert('not yet implemented');"></i>
                                </p>
                            </th>
                        </g:each>
                    </g:each>
                    <th class="text-center">
                        <br/>
                        <br/>
                        <br/>
                        <i title="add column" class="fa fa-plus fa-2x" onclick="alert('not yet implemented');"></i>
                    </th>
                </tr>
            </thead>
            <tbody id="fcsTbl">
                    <g:render template="annotationTmpl/tablTmpl" />
            </tbody>
        </table>
        <script>
            function eMetaClick(id,checked, mId){
                var metaVal = document.getElementById("eMeta_"+mId+".mdVal").value;

                $.ajax({
                    url: "${g.createLink(controller: 'expFile', action: 'axMetaSelect')}",
                    dataType: 'json',
                    data: {id: id, checked: checked, mId: mId, metaVal: metaVal},
                    success:  function (data, status, xhr){
                        console.log('success');
                        $("#tblCell_"+id.toString()+"_"+mId.toString()).html(data.cellTabl);
                    },
                    error: function(request, status, error){
                        console.log('ajxError!');
                    },
                    complete: function(xhr, status){
                        console.log('ajxComplete!');
                    }
                });

            }

            function eMetaValueChange(mId, valId){
                $.ajax({
                    url: "${g.createLink(controller: 'expFile', action: 'axMetaChange') }",
                    dataType: 'json',
                    data: {id: mId, valId: valId},
                    success:  function (data, status, xhr){
                        console.log('success');
                        $("#fcsTbl").html(data.tablTabl);
                    },
                    error: function(request, status, error){
                        console.log('ajxError!');
                    },
                    complete: function(xhr, status){
                        console.log('ajxComplete!');
                    }
                });
            }
        </script>
        %{--
        <div class="pagination">
            <g:paginate total="${plateCount ?: 0}" />
        </div>
        --}%

                %{--</div>--}%
            %{--</div>--}%
        %{--</div>--}%
    </div>
</div>