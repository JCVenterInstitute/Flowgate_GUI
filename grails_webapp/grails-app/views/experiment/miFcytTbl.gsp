<%@ page import="flowgate.ExperimentMetadataCategory" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment')}"/>
  <title><g:message code="default.mifcyt.annotation.label" default="MIFlowCyt Annotation" /></title>
  <style>
  .noLinkBlack {
    color: black;
    text-decoration: none;
  }

  .mifcytTable {
    padding-left: 40px;
    padding-right: 40px;
  }
  </style>
</head>
<body>
<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="mifcytTable-annotation" class="content scaffold-list" role="main">
      <ul class="breadcrumb">
        <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}"
               title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
        <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
        <li class="active"><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></li>
      </ul>

      <h1 class="page-header"><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></h1>

          <div class="row">
            <div class="col-sm-2">
              <div class="input-group">
                <label class="input-group-addon" for="download-template">Download Template</label>
                <span class="input-group-btn">
                  <a id="download-template" class="btn btn-sm btn-warning" href="${createLink(controller: 'experiment', action: 'exportMifCytTempl', id: experiment?.id)}">
                    <span class="fa fa-save"></span>
                  </a>
                </span>
              </div>
            </div>
            <div class="col-sm-offset-2 col-sm-8">
              <form id="upldForm" action="${g.createLink(controller: 'experiment', action: 'importMifcyt', id: experiment?.id)}" method="post" enctype="multipart/form-data">
                <div class="form-row" style="display: flex;align-items: center;">
                  <label>File Type:&nbsp;</label>
                  <div class="input-group input-group-sm">
                    <g:radioGroup name="separator"
                                  labels="['CSV (,)', 'TSV (Tab)']"
                                  values="[',', '\t']"
                                  value=",">
                      ${it.radio}&nbsp;<span>${it.label}</span>&nbsp;
                    </g:radioGroup>&nbsp;
                    <span class="input-group-addon" id="upload-file-info">No file Selected</span>
                    <span class="input-group-btn">
                      <label class="btn btn-sm btn-default" for="mifcytFile">
                        <input type="file" id="mifcytFile" name="mifcytFile" accept=".csv,.tsv" required="required" style="display:none;"
                               onchange="$('#upload-file-info').html(this.files[0].name)"/>
                        Select MIFlowCyt File
                      </label>
                      <input id="submitBtn" class="btn btn-sm btn-primary" type="submit" value="Upload" onclick="submitClick();"/>
                    </span>
                  </div>

                  <script>
                    function submitClick() {
                      $("#submitBtn").attr("disabled", true);
                      $("#upldForm").submit();
                      $("#clickMsg").html('<p class="alert-info">Uploading MIFlowCyt file ... </p>');
                    }
                  </script>
                </div>
              </form>
%{--              <div class="pull-right">--}%
%{--                <g:link class="btn btn-success" controller="experiment" action="index" params="[eId: experiment?.id]">--}%
%{--                <g:link class="btn btn-success" controller="experiment" action="saveMiFlowData" params="[eId: experiment?.id]">--}%
%{--                  <g:message code="submitBtn.label" default="Save and Return"/>--}%
%{--                </g:link>--}%
%{--                <g:actionSubmit class="btn btn-success" value="Save and Return" action="saveMiFlowData" />--}%
%{--              </div>--}%
            </div>
          </div>
          <g:if test="${flash.msg}">
            <div class="alert alert-warning">${flash.msg}</div>
          </g:if>
          <br/>
          <div class="row" style="max-width: none">
            <div class="col-sm-12 mifcytTable">

              <script>
                $(function () {
                  $(".scroll-wrapper").width($("#wholeTbl").width());
                  $(".scroll-top").width($("#mifcyt-annotation-table").width());
                  $(".scroll-wrapper").scroll(function () {
                    $("#wholeTbl").scrollLeft($(".scroll-wrapper").scrollLeft());
                  });
                  $("#wholeTbl").scroll(function () {
                    $(".scroll-wrapper").scrollLeft($("#wholeTbl").scrollLeft());
                  });
                });
              </script>

              <g:render template="templates/mifcytMasterTbl" model="[category: null]"/>
          </div>
      </div>
    </div>
  </div>
</div>
%{--</div>--}%
%{--<script>
  function eMetaClick(id, checked, mId) {
    var metaVal = document.getElementById("eMeta_" + mId + ".mdVal").value;

    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axMetaSelect')}",
      dataType: 'json',
      data: {id: id, checked: checked, mId: mId, metaVal: metaVal},
      success: function (data, status, xhr) {
        console.log('success');
        $("#tblCell_" + id.toString() + "_" + mId.toString()).html(data.cellTabl);
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });

  }

  function addColClick(eId, category) {
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axAddColumn') }",
      dataType: 'json',
      data: {id: eId, category: category},
      success: function (data, status, xhr) {
        console.log('success');
        $("#colCreateModal").html(data.crModalTmpl);
        $("#addColForm").modal("show");
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });
  }

  function addCategoryClick(eId) {
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axAddCategory') }",
      dataType: 'json',
      data: {id: eId},
      success: function (data, status, xhr) {
        console.log('success');
        $("#categoryAddModal").html(data.catModalTmpl);
        $("#addCategoryForm").modal("show");
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });

  }

  function editCategoryClick(eId, catId) {
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axEditCategory') }",
      dataType: 'json',
      data: {id: eId, catId: catId},
      success: function (data, status, xhr) {
        console.log('success');
        $("#categoryEditModal").html(data.catModalTmpl);
        $("#editCategoryForm").modal("show");
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });

  }

  function addValClick() {
    // var metaVal = document.getElementById("eMeta_"+mId+".mdVal").value;

    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axMetaValAdd')}",
      dataType: 'json',
      data: {},
      success: function (data, status, xhr) {
        console.log('metaValAdd success');
        $("#oldRow").replaceWith(data.expMetaDatOldRow);
        $("#newRow").replaceWith(data.expMetaDatNewRow);
      },
      error: function (request, status, error) {
        console.log('metaValAdd ajxError!');
      },
      complete: function (xhr, status) {
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

  function eMetaActionChange(mId, colAction) {
    switch (colAction) {
      case 'Delete':
        if (confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}')) {
          $.ajax({
            url: "${g.createLink(controller: 'expFile', action: 'axMetaActionChange') }",
            dataType: 'json',
            data: {id: mId, colAction: colAction},
            success: function (data, status, xhr) {
              console.log('success');
              $("#wholeTbl").html(data.tablTabl);
            },
            error: function (request, status, error) {
              console.log('ajxError!');
            },
            complete: function (xhr, status) {
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
          success: function (data, status, xhr) {
            console.log('success');
            $("#colEditModal").html(data.edModalTmpl);
            $("#editColForm").modal({show: 'true'});
          },
          error: function (request, status, error) {
            console.log('ajxError!');
          },
          complete: function (xhr, status) {
            console.log('ajxComplete!');
          }
        });
        break;

      default:
        $.ajax({
          url: "${g.createLink(controller: 'expFile', action: 'axMetaActionChange') }",
          dataType: 'json',
          data: {id: mId, colAction: colAction},
          success: function (data, status, xhr) {
            console.log('success');
            $("#wholeTbl").html(data.tablTabl);
          },
          error: function (request, status, error) {
            console.log('ajxError!');
          },
          complete: function (xhr, status) {
            console.log('ajxComplete!');
          }
        });
    }
  }

  function showAllHidden(eId, catId) {
    $.ajax({
      url: "${g.createLink(controller: 'expFile', action: 'axShowAllCols') }",
      dataType: 'json',
      data: {id: eId, category: catId},
      success: function (data, status, xhr) {
        console.log('success');
        $("#wholeTbl").html(data.tablTabl);
      },
      error: function (request, status, error) {
        console.log('ajxError!');
      },
      complete: function (xhr, status) {
        console.log('ajxComplete!');
      }
    });
  }
</script>--}%
</body>
</html>
