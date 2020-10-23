<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
  <asset:javascript src="jquery/jquery.dataTables.js"/>
  <script type="text/javascript" src="https://cdn.datatables.net/1.10.20/js/dataTables.material.min.js"></script>
  <link rel="stylesheet" href="https://cdn.datatables.net/1.10.20/css/dataTables.material.min.css">
  <asset:javascript src="spring-websocket"/>
</head>

<body>
<div class="navigation nav-wrapper">
  <div class="col s12">
    <span class="breadcrumb dark" style="cursor: pointer;"><i id="breadcrumbs_unfold" class="material-icons" style="margin-left: -15px;">unfold_more</i></span>
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="#!" class="breadcrumb dark">Analysis Tasks</a>
  </div>
</div>

<g:if test="${flash?.eMsg}">
  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var messageToastHTML = '<span>${flash?.eMsg}</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>';
      M.toast({
        html: messageToastHTML,
        displayLength: 8000
      });
    });
  </script>
</g:if>

<h2><g:message code="analysis.task.list.label" default="List of Analysis Tasks"/></h2>

<div id="analysisListTabl">
  <g:render template="templates/analysisListTbl" model="[]"/>
</div>

<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: params?.eId])}"
     data-tooltip="Create a new analysis" data-position="left">
    <i class="material-icons">add</i>
  </a>
</div>

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

<script>
  function showResult() {
    window.open("${g.createLink(controller: 'experiment', action: 'renderAnalysisHtml2')}", "resultsFrame")
  }

  jQuery.extend(jQuery.fn.dataTableExt.oSort, {
    "date-euro-pre": function (a) {
      var x;

      if ($.trim(a) !== '') {
        var frDatea = $.trim(a).split(' ');
        var frTimea = (undefined != frDatea[1]) ? frDatea[1].split(':') : [00, 00, 00];
        var frDatea2 = frDatea[0].split('/');
        x = (frDatea2[2] + frDatea2[1] + frDatea2[0] + frTimea[0] + frTimea[1] + ((undefined != frTimea[2]) ? frTimea[2] : 0)) * 1;
      } else {
        x = Infinity;
      }

      return x;
    },

    "date-euro-asc": function (a, b) {
      return a - b;
    },

    "date-euro-desc": function (a, b) {
      return b - a;
    }
  });

  function setCurrentTime() {
    var now = new Date;
    var seconds = now.getSeconds();
    var minutes = now.getMinutes();
    var hours = now.getHours();
    var ampm = hours >= 12 ? 'pm' : 'am';
    hours = hours % 12;
    hours = hours ? hours : 12;
    minutes = minutes < 10 ? '0' + minutes : minutes;
    seconds = seconds < 10 ? '0' + seconds : seconds;

    $("#lastUpdatedTime").text("Last updated on " + hours + ":" + minutes + ":" + seconds + " " + ampm);
  }

  var analysisTable;
  $(document).ready(function () {

    analysisTable = $("#analysis-table").DataTable({
      "columnDefs": [
        {"type": "date-euro", targets: 2},
        {
          targets: [ 0, 1, 2 ],
          className: 'mdl-data-table__cell--non-numeric'
        }
      ],
      "order": [[2, "desc"]]
    });
    setCurrentTime();
  });
</script>

<script type="text/javascript">

  var socket = new SockJS("${createLink(uri: '/stomp')}");
  var client = Stomp.over(socket);
  var interval = null;

  client.connect({}, function () {
    client.subscribe("/topic/taskChange", function (message) {
      console.log("received msg from service! ", message);
      var jobNo = JSON.parse(message.body).jobNo;
      console.log(jobNo);
      if ($('.modal.in').length <= 0) {
        // no modal open, set timer for status change / page reload
        interval = setInterval(gotStatusChange, 1000);
        console.log('initiated page reload');

      } else {
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
    window.location.href = "<g:createLink controller="analysis" action="index" params="[eId: params?.eId,offset: params?.offset]"/>";
    setCurrentTime();
  }

  function checkForMissedEvents() {
    if (localStorage.getItem('gotStatusChange') && interval === null) {
      // got status change while modal was open
      gotStatusChange();
    }
  }

</script>
</body>
</html>
