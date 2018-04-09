<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}" />
  %{--<title><g:message code="default.expFile.annotation.label" default="Manage Dataset" args="[entityName]" /></title>--}%
  <title><g:message code="default.expFile.annotation.label" default="Manage Dataset" /></title>
  <asset:javascript src="jquery-2.2.0.min.js"/>
  <style>
    #toAnno, #toFcs{
        background-color: rgba(122, 122, 122, 0.27);
    }
  </style>
</head>

<body>
<g:render template="/shared/nav"/>
<div class="nav" role="navigation">
</div>
%{--
experiment=${experiment} id=${experiment?.id} ${experiment == null}
project=${experiment?.project}
--}%

<g:if test="${experiment}">
  <div class="text-center">Datasets - ${experiment?.title}</div>
  <br/>
  <g:form controller="dataset" action="dsUpdate" params="[experiment1: experiment]" id="${experiment.id}">
    <g:hiddenField name="expId" value="${experiment?.id}" />
    <g:hiddenField name="projId" value="${experiment?.project?.id}" />
    <div class="row" style="padding-left: 20px;padding-right:20px;max-width: 100%">
      <div class="col-sm-2" style="padding-right: 0">
          %{--<content tag="fcsPanel">--}%
          <div id="metaData" >
            <g:render template="datasetTmpl/mdFilterPanel" model="[experiment: experiment]" />
          </div>
          %{--</content>--}%
        </div>
        <div class="col-sm-2" style="padding-left: 0">
          %{--<content tag="fcsPanel">--}%
          <div id="fcsPanel">
            <g:render template="datasetTmpl/fcsFilePanel" model="[experiment: experiment, ds:ds, dsId: ds.id ]" />
          </div>
          %{--</content>--}%
        </div>
        <div class="col-sm-1">
          <div id="datasetBtnPnl" style="padding-top: 150px">
            <p><div id="toDs" class="btn btn-default" onclick="alert('not implemented yet!');" style="width: 100%" ><i class="fa fa-caret-right" ></i></div></p>
            %{--<p>&nbsp</p>--}%
            %{--<p><button id="toFcs" type="submit" value="action_left" class="btn btn-default" style="width: 100%" ><i class="fa fa-caret-left" ></i></button></p>--}%
          </div>
        </div>
        <div class="col-sm-4" >
          %{--<content tag="pgContent">--}%
            <div id="dsPanel">
            %{--<div class="pull-right" style="padding-right: 30px">--}%
              <div class="btn btn-default" onclick="addDs(${experiment.id});"><i class="fa fa-plus" ></i>Add New Dataset</div>
            %{--</div>--}%
              <p></p>
              <g:render template="datasetTmpl/datasetPanel" model="[experiment: experiment, dsMode: params.dsMode]" />
            </div>
            <br/>
            <br/>
            <br/>
            <div class="text-center">
              <button type="submit" class="btn btn-success"  >Submit</button>
            </div>
          %{--</content>--}%
        </div>
        <div class="col-sm-3" >
          %{--<content tag="pgRightPanel">--}%
            <br/>
            <br/>
            <br/>
            %{--<div class="text-center" style="padding-right: 20px">--}%
            <div class="text-center" >
              %{--<a class="noLinkBlack btn btn-default" href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment?.id)}"><i class="fa fa-table" ></i>&nbsp;Overview Table</a>--}%
            </div>
            <br/>
            <br/>
            <br/>
          %{--</content>--}%
        </div>
      </div>
    </g:form>
  </g:if>
  <script>
    function addDs(eId) {
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axAddDs')}",
        dataType: "json",
        type: "get",
        %{--data: {eId: ${params?.eId}, modId: moduleId},--}%
        data: {eId: eId},
        success: function (data) {
          $("#datasetField").html(data.dsField);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }
  </script>
</body>
</html>