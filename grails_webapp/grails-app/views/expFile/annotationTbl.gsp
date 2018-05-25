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
    .annotationTable{
      padding-left: 40px;
      padding-right: 40px;
    }
  </style>
</head>

<body>
<g:render template="/shared/nav" />
<div class="nav" role="navigation"></div>
<h2 class="text-center"><g:message code="annotation.table.label" default="Annotation Table" /></h2>
<br/>
<div class="row" style="max-width: none">
  <div class="col-sm-12 annotationTable">
    <div class="pull-right">
      <g:link class="btn btn-success" controller="experiment" action="index" params="[eId: experiment.id]"><g:message code="submitBtn.label" default="Submit"/></g:link>
    </div>
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
        <g:render template="annReagentsMasterTbl" model="[category: 'Reagents']" />
      </div>
      %{--
      <div class="tab-pane" role="tabpanel" id="tabPanel">--}%
        %{--<g:render template="annMasterTbl" model="[category: 'Panel']" />--}%
      %{--</div>
      --}%
    </div>
  </div>
  <div id="colCreateModal">
    <g:render template="annotationTmpl/colCreateModal" model="[experiment: experiment]"/>
  </div>
  <div id="colEditModal">
    %{--<g:render template="annotationTmpl/colEditModal" model="[experiment: experiment, eMeta: eMeta]"/>--}%
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

  function eMetaValueChange(mId, valId) {
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axMetaChange') }",
      dataType: 'json',
      data: {id: mId, valId: valId},
      success: function (data, status, xhr) {
        console.log('success');
        $("#fcsTbl").html(data.tablTabl);
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });
  }

  function eMetaActionChange(mId, colAction){
    switch(colAction){
      case 'Delete':
        if (confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}')){
          $.ajax({
            url: "${g.createLink(controller: 'expFile', action: 'axMetaActionChange') }",
            dataType: 'json',
            data: {id: mId, colAction: colAction},
            success:  function (data, status, xhr){
              console.log('success');
              $("#wholeTbl").html(data.tablTabl);
            },
            error: function(request, status, error){
              console.log('ajxError!');
            },
            complete: function(xhr, status){
              console.log('ajxComplete!');
            }
          });
        }
        break;
      case 'Edit':
          $.ajax({
            url: "${g.createLink(controller: 'expFile', action: 'axMetaActionChange') }",
            dataType: 'json',
            data: {id: mId, colAction: colAction},
            success:  function (data, status, xhr){
              console.log('success');
              $("#colEditModal").html(data.edModalTmpl);
            },
            error: function(request, status, error){
              console.log('ajxError!');
            },
            complete: function(xhr, status){
              console.log('ajxComplete!');
            }
          });
        break;

      default:
        $.ajax({
          url: "${g.createLink(controller: 'expFile', action: 'axMetaActionChange') }",
          dataType: 'json',
          data: {id: mId, colAction: colAction},
          success:  function (data, status, xhr){
            console.log('success');
            $("#wholeTbl").html(data.tablTabl);
          },
          error: function(request, status, error){
            console.log('ajxError!');
          },
          complete: function(xhr, status){
            console.log('ajxComplete!');
          }
        });
    }
  }

  function showAllHidden(eId, category){
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axShowAllCols') }",
      dataType: 'json',
      data: {id: eId, category: category},
      success:  function (data, status, xhr){
        console.log('success');
        $("#wholeTbl").html(data.tablTabl);
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