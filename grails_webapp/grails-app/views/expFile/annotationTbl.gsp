<%@ page import="flowgate.ExperimentMetadataCategory" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}" />
  <title><g:message code="default.expFile.annotation.label" default="Annotation" args="[entityName]" /></title>
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
    <g:if test="${experiment.expFiles}" >
      <form id="upldForm" action="${g.createLink(controller: 'expFile', action: 'importAnnotation', id: experiment?.id)}" method="post" enctype="multipart/form-data" >
        <g:hiddenField name="pId" value="${plate?.id}" />
        <div class="row">
          <div class="col-sm-offset-1">
            Annotation CSV-file <input type="file" name="annotationFile" accept=".csv,.tsv" required="required"/>
            File Separator:&nbsp;
            <g:radioGroup name="separator"
                          labels="['CSV (,)','TSV (Tab)']"
                          values="[',','\t']"
                          value=",">
              %{--<p>${it.label} ${it.radio}</p>--}%
              ${it.label}&nbsp;${it.radio}&nbsp;&nbsp;&nbsp;&nbsp;
            </g:radioGroup>
          </div>
        </div>
        <br/>
        <div class="col-sm-offset-2" >
          <input id="submitBtn" class="btn btn-success" type="submit" value="Upload" onclick="submitClick();" />
        </div>
        <script>

          function submitClick() {
            $("#submitBtn").attr("disabled", true);
            $("#upldForm").submit();
            $("#clickMsg").html('<p class="alert-info">Uploading annotation file ... </p>');
          }

        </script>
      </form>
    %{--<g:set var="categories" value="${((experiment.expMetadatas*.mdCategory).unique()-'Reagents')}" />--}%
    <g:set var="categories" value="${ExperimentMetadataCategory.findAllByExperiment(experiment)}" />
      %{--cats=${categories}--}%
    <ul class="nav nav-tabs">
      <g:if test="${categories.size()>0}">
        <g:each in="${categories}" var="category" status="idx" >
          <li class="${idx==0 ? 'active' : ''}"><a href="#tab${category.mdCategory}" role="tab" data-toggle="tab" ondblclick="alert('double click');" >${category.mdCategory}</a></li>
        </g:each>
      </g:if>
      <g:else>
        <li class="active"><a href="#tabBasics" role="tab" data-toggle="tab" ondblclick="alert('double click');" >Basics</a></li>
      </g:else>
      <li class="text-center">
        %{--<a href="" onclick="addCategoryClick(${experiment.id});" style="color: black;" role="tab"><i class="fa fa-plus"></i></a>--}%
        <div class="text-center" onclick="addCategoryClick(${experiment?.id});" style="color: black;padding-top: 12px;padding-left: 8px;" ><i title="add tab" class="fa fa-plus" ></i></div>
      </li>
      %{--<li><a href="#tabReagents" role="tab" data-toggle="tab">Reagents</a></li>--}%
    </ul>
    <div class="tab-content">
      <g:if test="${categories.size()>0}">
        <g:each in="${categories}" var="category" status="idx">
          <div class="tab-pane ${idx==0 ? 'active' : ''}" role="tabpanel" id="tab${category.mdCategory}" >
            <g:render template="annMasterTbl" model="[category: category]"/>
          </div>
        </g:each>
      </g:if>
      <g:else>
        <div class="tab-pane active" role="tabpanel" id="tabBasics" >
          <g:render template="annMasterTbl" model="[category: null]"/>
        </div>
      </g:else>
      %{--<div class="tab-pane" role="tabpanel" id="tabReagents">--}%
        %{--<g:render template="annReagentsMasterTbl" model="[category: 'Reagents']" />--}%
      </div>
    </div>
    </g:if>
    <g:else>
      <div class="text-center" >No FCS-files uploaded so far, please upload some FCS-files.</div>
    </g:else>
  </div>
  <div id="colEditModal"></div>
  <div id="colCreateModal"></div>
  <div id="categoryAddModal"></div>
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

  function addColClick(eId, category){
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axAddColumn') }",
      dataType: 'json',
      data: {id: eId, category: category},
      success:  function (data, status, xhr){
        console.log('success');
        $("#colCreateModal").html(data.crModalTmpl);
        $("#addColForm").modal("show");
      },
      error: function(request, status, error){
        console.log('ajxError!');
      },
      complete: function(xhr, status){
        console.log('ajxComplete!');
      }
    });
  }

  function addCategoryClick(eId){
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axAddCategory') }",
      dataType: 'json',
      data: {id: eId},
      success:  function (data, status, xhr){
        console.log('success');
        $("#categoryAddModal").html(data.catModalTmpl);
        $("#addCategoryForm").modal("show");
      },
      error: function(request, status, error){
        console.log('ajxError!');
      },
      complete: function(xhr, status){
        console.log('ajxComplete!');
      }
    });

  }

  function addValClick(){
    // var metaVal = document.getElementById("eMeta_"+mId+".mdVal").value;

    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axMetaValAdd')}",
      dataType: 'json',
      data: {},
      success:  function (data, status, xhr){
        console.log('metaValAdd success');
        $("#oldRow").replaceWith(data.expMetaDatOldRow);
        $("#newRow").replaceWith(data.expMetaDatNewRow);
      },
      error: function(request, status, error){
        console.log('metaValAdd ajxError!');
      },
      complete: function(xhr, status){
        console.log('metaValAdd ajxComplete!');
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
              $("#editColForm").modal({ show: 'true' });
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

  function showAllHidden(eId, catId){
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axShowAllCols') }",
      dataType: 'json',
      data: {id: eId, category: catId},
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