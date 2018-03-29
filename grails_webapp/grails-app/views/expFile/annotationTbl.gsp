<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}" />
        <title><g:message code="default.expFile.annotation.label" default="Annotation" args="[entityName]" /></title>
        <asset:javascript src="jquery-2.2.0.min.js"/>
        <style>
            .noLinkBlack {
                color: black;
                text-decoration: none;
            }
            /* table with horizontal scroll */
            /*
            table {
                !*width: 100%;*!
                display: block;
                overflow-x: auto;
            }
            */
        </style>
    </head>
    <body>
        <g:render template="/shared/nav" />
        <h2 class="text-center"><g:message code="annotation.table.label" default="Annotation Table" /></h2>
        <br/>
        <div class="row" style="max-width: none">
            <div class="col-sm-12">
                <ul class="nav nav-tabs">
                    <li class="active"><a href="#tabDemogr" role="tab" data-toggle="tab">Demographics</a></li>
                    <li><a href="#tabVisit" role="tab" data-toggle="tab">Visit</a></li>
                    <li><a href="#tabStimulation" role="tab" data-toggle="tab">Stimulation</a></li>
                    <li><a href="#tabReagents" role="tab" data-toggle="tab">Reagents</a></li>
                </ul>
                <div class="tab-content">
                    <div class="tab-pane active" role="tabpanel" id="tabDemogr">
                        <g:render template="annMasterTbl" model="[category: 'Demographics']"/>
                    </div>
                    <div class="tab-pane" role="tabpanel" id="tabVisit" >
                        <g:render template="annMasterTbl" model="[category: 'Visit']" />
                    </div>
                    <div class="tab-pane" role="tabpanel" id="tabStimulation">
                        <g:render template="annMasterTbl" model="[category: 'Stimulation']" />
                    </div>
                    <div class="tab-pane" role="tabpanel" id="tabReagents">
                        <g:render template="annMasterTbl" model="[category: 'Reagents']" />
                    </div>
                </div>
            </div>
        </div>
        <br/>
        <br/>
        <div class="text-center">
            %{--<g:link controller="expFile" action="doneAnnotation" id="${experiment.id}"><div class="btn btn-success">Submit</div></g:link>--}%
            <g:link controller="experiment" action="index" id="${experiment.id}" params="[eId: experiment.id]"><div class="btn btn-success">Submit</div></g:link>
        </div>
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
    </body>
</html>