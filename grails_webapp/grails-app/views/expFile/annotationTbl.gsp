<%@ page import="flowgate.ExperimentMetadataCategory" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}"/>
  <title><g:message code="default.expFile.annotation.label" default="Annotation" args="[entityName]"/></title>
  <style>
  .noLinkBlack {
    color: black;
    text-decoration: none;
  }

  .annotationTable {
    padding-left: 40px;
    padding-right: 40px;
  }
  </style>
</head>

<body>
<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="annotation-analysis" class="content scaffold-list" role="main">
      <ul class="breadcrumb">
        <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}"
               title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
        <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
        <li class="active"><g:message code="annotation.table.label" default="Annotate Uploaded FCS Files with Experiment Metadata"/></li>
      </ul>

      <h1 class="page-header"><g:message code="annotation.table.label" default="Annotate Uploaded FCS Files with Experiment Metadata"/></h1>

      <p>The annotation is based on individual FCS files by user providing attribute categories, attribute names, and attribute values. The procedure consists of two steps:</p>
      <dl class="dl-horizontal">
        <dt>Step 1</dt>
        <dd>Upload a CSV (comma-delimited) or a TSV (tab-delimited) file that contains the metadata. Each row is one FCS file and each column is one attribute.
        The first column is the name of the FCS file. The name of the first column must be “FCS File Name”, and the names of the attributes in the rest of the header.
        If the metadata table is small, you can also skip the Step 1 and manually create the metadata table in <b>Step 2.</b></dd>
        <br/>
        <dt></dt>
        <dd>
          <div class="row">
            <div class="col-sm-2">
              <div class="input-group">
                <label class="input-group-addon" for="download-template">Download Template</label>
                <span class="input-group-btn">
                  <a id="download-template" class="btn btn-sm btn-warning" href="${createLink(controller: 'expFile', action: 'exportAnnotationTempl', id: experiment?.id)}">
                    <span class="fa fa-save"></span>
                  </a>
                </span>
              </div>
            </div>
            <div class="col-sm-offset-2 col-sm-8">
              <form id="upldForm" action="${g.createLink(controller: 'expFile', action: 'importAnnotation', id: experiment?.id)}" method="post" enctype="multipart/form-data">
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
                      <label class="btn btn-sm btn-default" for="annotationFile">
                        <input type="file" id="annotationFile" name="annotationFile" accept=".csv,.tsv" required="required" style="display:none;"
                               onchange="$('#upload-file-info').html(this.files[0].name)"/>
                        Select Annotation File
                      </label>
                      <input id="submitBtn" class="btn btn-sm btn-primary" type="submit" value="Upload" onclick="submitClick();"/>
                    </span>
                  </div>

                  <script>
                    function submitClick() {
                      $("#submitBtn").attr("disabled", true);
                      $("#upldForm").submit();
                      $("#clickMsg").html('<p class="alert-info">Uploading annotation file ... </p>');
                    }
                  </script>
                </div>
              </form>
            </div>
          </div>
        </dd>
        <br/>
        <dt>Step 2</dt>
        <dd>Create a new or edit an existing metadata table (e.g., from user-uploaded metadata table)</dd>
        <dt></dt>
        <dd>By clicking “+” on the tab, a new attribute category will be created. The user can specify the category of an attribute when creating an attribute</dd>
        <dt></dt>
        <dd>By clicking “+” in the annotation table, a new attribute will be created. The user can specify its name, category, and possible values</dd>
        <dt></dt>
        <dd>After providing the possible values of an attribute, the user does not need to type in its value in the table, but just select the value from the drop down list and check/uncheck the corresponding checkbox for the FCS file.</dd>
      </dl>

      <div class="row" style="max-width: none">
      <div class="col-sm-12 annotationTable">
        <div class="pull-right">
          <g:link class="btn btn-success" controller="experiment" action="index" params="[eId: experiment.id]"><g:message code="submitBtn.label"
                                                                                                                          default="Save and Return"/></g:link>
        </div>
        <g:if test="${experiment.expFiles}">
        %{--<g:set var="categories" value="${((experiment.expMetadatas*.mdCategory).unique()-'Reagents')}" />--}%
          <g:set var="categories" value="${ExperimentMetadataCategory.findAllByExperiment(experiment)}"/>
        %{--cats=${categories}--}%
          <ul class="nav nav-tabs">
            <g:if test="${categories.size() > 0}">
              <g:each in="${categories}" var="category" status="idx">
                <li class="${idx == 0 ? 'active' : ''}"><a href="#tab${category?.mdCategory}" role="tab" data-toggle="tab"
                                                           ondblclick="editCategoryClick(${experiment.id}, ${category?.id});">${category?.mdCategory}</a></li>
              </g:each>
            </g:if>
            <g:else>
              <li class="active"><a href="#tabBasics" role="tab" data-toggle="tab" ondblclick="editCategoryClick(${experiment?.id}, ${category?.id});">Basics</a></li>
            </g:else>
            <li class="text-center">
              %{--<a href="" onclick="addCategoryClick(${experiment.id});" style="color: black;" role="tab"><i class="fa fa-plus"></i></a>--}%
              <div class="text-center" onclick="addCategoryClick(${experiment?.id});" style="color: black;padding-top: 12px;padding-left: 8px;"><i title="add tab" class="fa fa-plus"></i></div>
            </li>
            %{--<li><a href="#tabReagents" role="tab" data-toggle="tab">Reagents</a></li>--}%
          </ul>

          <div class="tab-content">
          <script>
            $(function () {
              $(".scroll-wrapper").width($("#wholeTbl").width());
              $(".scroll-top").width($("#annotation-table").width());
              $(".scroll-wrapper").scroll(function () {
                $("#wholeTbl").scrollLeft($(".scroll-wrapper").scrollLeft());
              });
              $("#wholeTbl").scroll(function () {
                $(".scroll-wrapper").scrollLeft($("#wholeTbl").scrollLeft());
              });
            });
          </script>

          <div class="scroll-wrapper" style="overflow-x: scroll; overflow-y:hidden;">
            <div class="scroll-top" style="height: 1px;"></div>
          </div>
            <g:if test="${categories.size() > 0}">
              <g:each in="${categories}" var="category" status="idx">
                <div class="tab-pane ${idx == 0 ? 'active' : ''}" role="tabpanel" id="tab${category?.mdCategory}">
                  <g:render template="annMasterTbl" model="[category: category]"/>
                </div>
              </g:each>
            </g:if>
            <g:else>
              <div class="tab-pane active" role="tabpanel" id="tabBasics">
                <g:render template="annMasterTbl" model="[category: null]"/>
              </div>
            </g:else>
          %{--<div class="tab-pane" role="tabpanel" id="tabReagents">--}%
          %{--<g:render template="annReagentsMasterTbl" model="[category: 'Reagents']" />--}%
          </div>
          </div>
        </g:if>
        <g:else>
          <div class="text-center">No FCS-files uploaded so far, please upload some FCS-files.</div>
        </g:else>
      </div>

      <div id="colEditModal"></div>

      <div id="colCreateModal"></div>

      <div id="categoryAddModal"></div>

      <div id="categoryEditModal"></div>
    </div>
  </div>
</div>
</div>
<script>
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
</script>
</body>
</html>
