<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}" />
  <title><g:message code="default.expFile.annotation.label" default="Manage Dataset" /></title>
  <asset:javascript src="jquery-2.2.0.min.js"/>
</head>

<body>
<g:render template="/shared/nav"/>
<div class="nav" role="navigation">
</div>
<g:if test="${experiment}">
  <div class="text-center">Datasets - ${experiment?.title}</div>
  <br/>
  <g:form controller="dataset" action="dsExit" params="[experiment: experiment]" id="${experiment.id}">
    <g:hiddenField name="expId" value="${experiment?.id}" />
    <g:hiddenField name="projId" value="${experiment?.project?.id}" />
    <div class="row" style="padding-left: 20px;padding-right:20px;max-width: 100%">
      <div class="col-sm-offset-1 col-sm-2" style="padding-right: 0">
          <div id="metaData" >
            <g:render template="datasetTmpl/mdFilterPanel" model="[experiment: experiment]" />
          </div>
        </div>
        <div class="col-sm-2" style="padding-left: 0">
          <div id="fcsCandidates">
            <g:render template="datasetTmpl/fcsFileCandidates" model="[experiment: experiment, ds:ds, dsId: ds.id, expFileCandidatesList: expFileCandidatesList, pType: 'cand' ]" />
          </div>
        </div>
        <div class="col-sm-1">
          <div id="datasetBtnPnl" style="padding-top: 150px">
            <p><g:actionSubmit id="toDs" class="btn btn-default" style="width: 100%" action="assign" value="assign" /></p>
            <p><g:actionSubmit id="fromDs" class="btn btn-default" style="width: 100%" action="remove" value="remove" /></p>
          </div>
        </div>
        <div class="col-sm-2" style="">
            <div id="dsPanel">
                <g:render template="datasetTmpl/datasetPanel" model="[experiment: experiment, dsMode: params.dsMode]" />
            </div>
            <div id="fcsAssigned">
              <g:render template="datasetTmpl/fcsFileAssigned" model="[experiment: experiment, ds:ds, dsId: ds.id, expFileAssignedList: ds.expFiles, pType: 'ass' ]" />
            </div>
        </div>
      <div class="col-sm-1">
        <div id="submitBtnPnl" style="padding-top: 150px">
          <div class="pull-right">
            <button type="submit" class="btn btn-success"  >Submit</button>
          </div>
        </div>
      </div>
      </div>
    </g:form>
  </g:if>
  <script type="text/javascript">
    function addDs(dsId) {
      var eId = ${experiment.id};
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axAddDs')}",
        dataType: "json",
        type: "get",
        data: {id: eId},
        success: function (data) {
          $("#datasetField").html(data.dsField);
          $("#fcsAssigned").html(data.fcsAssList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function editDs(dsId) {
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axEditDs')}",
        dataType: "json",
        type: "get",
        data: {id: dsId},
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

    function setFilter(metaVal, ckStatus){
      var eId = ${experiment.id};
      var dsId = ${ds.id};
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axSetFilter')}",
        dataType: "json",
        data: {id: eId, dsId: dsId, eMetaVal: metaVal, ckStatus: ckStatus},
        type: "get",
        success: function (data) {
          $("#fcsCandidates").html(data.fcsList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function selAllCandFcs(dsId){
      var eId = ${experiment.id};
      var dsId = ${ds.id};
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axSelAllCandFcs')}",
        dataType: "json",
        data: {id: eId, dsId: dsId},
        type: "get",
        success: function (data) {
          $("#fcsCandidates").html(data.fcsCandList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function deselAllCandFcs(dsId){
      var eId = ${experiment.id};
      var dsId = ${ds.id};
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axDeselAllCandFcs')}",
        dataType: "json",
        data: {id: eId, dsId: dsId},
        type: "get",
        success: function (data) {
          $("#fcsCandidates").html(data.fcsCandList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function selAllAssFcs(dsId){
      %{--var eId = ${experiment.id};--}%
      %{--var dsId = ${ds.id};--}%
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axSelAllAssFcs')}",
        dataType: "json",
        data: {id: dsId},
        type: "get",
        success: function (data) {
          $("#fcsAssigned").html(data.fcsAssList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function deselAllAssFcs(dsId){
      %{--var eId = ${experiment.id};--}%
      %{--var dsId = ${ds.id};--}%
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axDeselAllAssFcs')}",
        dataType: "json",
        // data: {id: eId, dsId: dsId},
        data: {id: dsId},
        type: "get",
        success: function (data) {
          $("#fcsAssigned").html(data.fcsAssList);
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }

    function dsSelChange(dsId) {
      $.ajax({
        url: "${createLink(controller: 'dataset', action: 'axDsChange')}",
        dataType: "json",
        type: "get",
        %{--data: {eId: ${params?.eId}, modId: moduleId},--}%
        data: {id: dsId},
        success: function (data) {
          // alert("ds Changed!");
          $("#metaData").html(data.metaData);
          $("#dsPanel").html(data.dsPanel);
          // $("#fcsCandidates").html(data.fcsCandidates);
          $("#fcsAssigned").html(data.fcsAssigned);
          console.log('ds changed');
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
          // alert("ds Change error");
        },
        complete: function () {
          // alert("ds Change completed");
          console.log('ajax ds change completed');
        }
      });
    }
  </script>

</body>
</html>