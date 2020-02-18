<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
  <asset:javascript src="jquery/jquery.dataTables.js"/>
  <asset:stylesheet src="jquery.dataTables.css"/>

  <asset:javascript src="spring-websocket" />


  <style>
      .div-as-link{
        cursor: pointer;
        /*text-decoration: underline;*/
        color: #337ab7;
      }


    /*modal full screen  */


    .modal {
      position: fixed;
      top: 0;
      right: 0;
      bottom: 0;
      left: 0;
      overflow: hidden;
    }

    .modal-dialog {
      position: fixed;
      margin: 0;
      width: 100%;
      height: 100%;
      padding: 0;
    }

    .modal-content {
      /*
      height: auto;
      min-height: 100%;
      */
      position: absolute;
      top: 0;
      right: 0;
      bottom: 0;
      left: 0;
      border: 2px solid #3c7dcf;
      border-radius: 0;
      box-shadow: none;
    }

    .modal-header {
      position: absolute;
      top: 0;
      right: 0;
      left: 0;
      height: 50px;
      padding: 10px;
      background: #6598d9;
      border: 0;
    }

    .modal-title {
      font-weight: 300;
      font-size: 2em;
      color: #fff;
      line-height: 30px;
    }

    .modal-body {
      position: absolute;
      top: 50px;
      bottom: 60px;
      width: 100%;
      font-weight: 300;
      overflow: auto;
    }

    .modal-footer {
      position: absolute;
      right: 0;
      bottom: 0;
      left: 0;
      height: 60px;
      padding: 10px;
      background: #f1f3f5;
    }

    ::-webkit-scrollbar {
      -webkit-appearance: none;
      width: 10px;
      background: #f1f3f5;
      /*border-left: 1px solid darken(#f1f3f5, 10%);*/
    }

    ::-webkit-scrollbar-thumb {
      /*background: darken(#f1f3f5, 20%);*/
    }

  .sInit {
    color: lightgrey;
  }
  .sPending {
    color: orange;
  }
  .sResultsReady {
    color: lawngreen;
  }
  .sError {
    color: red;
  }
  .sDefault {
    color: brown;
  }

</style>
</head>

<body>
<div class="nav" role="navigation"></div>

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="list-analysis" class="content scaffold-list" role="main">
      <ul class="breadcrumb">
        <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
        <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
        <li class="active"><g:message code="analysis.task.list.label" default="List of Analysis Tasks" /></li>
      </ul>
      %{--
      <g:if env="development">
        <a class="noLinkBlack text-center" href="${g.createLink(controller: 'analysis', action: 'd3demo')}" >
          <div class="btn btn-default">D3-Demo</div>
        </a>
      </g:if>
      --}%
      <h1 class="page-header"><g:message code="analysis.task.list.label" default="List of Analysis Tasks" /></h1>
%{--      <div id="changeDiv">msgHere</div>--}%
      <div id="analysisListTabl">
        <g:render template="templates/analysisListTbl" model="[]"/>
      </div>
      %{--TODO do not reactivate after analysis list tests!!! is replaced --}%
%{--      <div id="chkScript">--}%
        %{--joblist=${jobList}--}%
%{--        <g:render template="templates/statusCheckTempl" model="[jobList: jobList]"/>--}%
%{--      </div>--}%
      <br/>
      <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: params?.eId])}">
        <button class="btn btn-primary"><span class="glyphicon glyphicon-plus"></span>&nbspNew Analysis</button>
      </a>
      <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_Acs">
        <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'setCallback', params: [eId: params?.eId])}">
          <button class="btn btn-primary"><span class="glyphicon glyphicon-phone-alt"></span>&nbspSet Callback</button>
        </a>
      </sec:ifAnyGranted>

%{--      <button class="btn btn-info" onclick="checkTimer()"><span class="glyphicon glyphicon-refresh"></span>&nbsp;Update task status</button> <label style="margin-left: 5px" id="lastUpdatedTime"></label>--}%

      <label style="margin-left: 5px" id="lastUpdatedTime"></label>
      <sec:ifAnyGranted roles="ROLE_Tester,ROLE_Acs">
        <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'heatM')}">
          <button class="btn btn-default">GenePattern Heatmap</button>
        </a>
        <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'fcs2Csv')}">
          <button class="btn btn-default">GenePattern Fcs2Csv</button>
        </a>
        <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'flock')}">
          <button class="btn btn-default">GenePattern flock</button>
        </a>
      %{--  Buttons to test results page  --}%
        <a href="javascript:void(0)" onclick="javascript:showResult()" width="200" height="200">
          <div class="btn btn-info">newHtml</div>
        </a>

        <div class="btn btn-info" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">newHtml2</div>
      </sec:ifAnyGranted>
    </div>
  </div>
</div>

<br/>
<br/>
<br/>
<g:if test="${flash?.eMsg}">
  <div class="text-center">
    <div class="alert alert-danger alert-dismissible" role="alert">
      <button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>
      <strong>GP error:</strong>&nbsp;&nbsp;${flash?.eMsg}
    </div>
  </div>
</g:if>

<script>
  function showResult() {
    window.open("${g.createLink(controller: 'experiment', action: 'renderAnalysisHtml2')}", "resultsFrame")
  }

  jQuery.extend( jQuery.fn.dataTableExt.oSort, {
    "date-euro-pre": function ( a ) {
      var x;

      if ( $.trim(a) !== '' ) {
        var frDatea = $.trim(a).split(' ');
        var frTimea = (undefined != frDatea[1]) ? frDatea[1].split(':') : [00,00,00];
        var frDatea2 = frDatea[0].split('/');
        x = (frDatea2[2] + frDatea2[1] + frDatea2[0] + frTimea[0] + frTimea[1] + ((undefined != frTimea[2]) ? frTimea[2] : 0)) * 1;
      }
      else {
        x = Infinity;
      }

      return x;
    },

    "date-euro-asc": function ( a, b ) {
      return a - b;
    },

    "date-euro-desc": function ( a, b ) {
      return b - a;
    }
  } );

  function setCurrentTime() {
    var now = new Date;
    var seconds = now.getSeconds();
    var minutes = now.getMinutes();
    var hours = now.getHours();
    var ampm = hours >= 12 ? 'pm' : 'am';
    hours = hours % 12;
    hours = hours ? hours : 12;
    minutes = minutes < 10 ? '0'+minutes : minutes;
    seconds = seconds < 10 ? '0'+seconds : seconds;

    $("#lastUpdatedTime").text("Last updated on " + hours + ":" + minutes + ":" + seconds + " " + ampm);
  }

  var analysisTable;
  $(document).ready(function () {

    analysisTable = $("#analysis-table").DataTable({
      "columnDefs": [
        { "type": "date-euro", targets: 2 }
      ],
      "order": [[ 2, "desc" ]]
    });
    setCurrentTime();
  });
</script>

<script type="text/javascript">

  var socket = new SockJS("${createLink(uri: '/stomp')}");
  var client = Stomp.over(socket);
  var interval = null;

  client.connect({}, function() {
    client.subscribe("/topic/taskChange", function(message) {
      console.log("received msg from service! ", message);
      var jobNo = JSON.parse(message.body).jobNo;
      console.log(jobNo);
      if ($('.modal.in').length <= 0) {
        // no modal open, set timer for status change / page reload
        interval = setInterval(gotStatusChange, 1000);
        console.log('initiated page reload');

      }
      else {
        // modal open store event to check when modal is closed
        localStorage.setItem('gotStatusChange', true);
        console.log('event muted / check on modal close');
      }
    });
  });

  function gotStatusChange() {
    clearInterval(interval);
    interval = null;
    console.log('in intervall loop');
    window.location.href = "${g.createLink(controller: 'analysis', action: 'index', params:[eId: params?.eId])}";
    setCurrentTime();
  }

  function checkForMissedEvents(){
    if(localStorage.getItem('gotStatusChange') && interval===null){
      // got status change while modal was open
      gotStatusChange();
    }
  }

</script>
</body>
</html>
