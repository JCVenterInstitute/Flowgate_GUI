<%@ page import="flowgate.ExperimentMetadataCategory" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment')}"/>
  <title><g:message code="default.mifcyt.annotation.label" default="MIFlowCyt Annotation" /></title>
  %{--
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
  --}%
</head>
<body>
<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="#!" class="breadcrumb dark"><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></a>
  </div>
</div>

<h2><g:message code="mifcyt.annotation.table.label" default="Annotate Experiment with MIFlowCyt data"/></h2>

%{--
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
--}%


<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped modal-trigger" href="#upload-annotation-file"
     data-tooltip="Upload an MIFlowCyt File" data-position="left">
    <i class="material-icons">file_upload</i>
  </a>
</div>

%{--
<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="${createLink(controller: 'experiment', action: 'exportMifCytTempl', id: experiment?.id)}"
     data-tooltip="Download template file" data-position="left">
    <i class="material-icons">file_upload</i>
  </a>
</div>
--}%

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="mifcytTable-annotation" class="content scaffold-list" role="main">
      <g:if test="${flash.error}">
        <script>
          document.addEventListener('DOMContentLoaded', function () {
            M.toast({
              html: '<span>${flash.error}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
              displayLength: Infinity,
              classes: 'red'
            });
          });
        </script>
      </g:if>
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
        %{--
        <!--
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
            </div>-->
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
<!--              <div class="pull-right">-->
<!--                <g:link class="btn btn-success" controller="experiment" action="index" params="[eId: experiment?.id]">-->
<!--                <g:link class="btn btn-success" controller="experiment" action="saveMiFlowData" params="[eId: experiment?.id]">-->
<!--                  <g:message code="submitBtn.label" default="Save and Return"/>-->
<!--                </g:link>-->
<!--                <g:actionSubmit class="btn btn-success" value="Save and Return" action="saveMiFlowData" />-->
<!--              </div>-->
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
          --}%
      </div>
    </div>
  </div>
</div>
</body>
</html>
