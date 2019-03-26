<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
  <asset:javascript src="jquery/jquery.dataTables.js"/>
  <asset:stylesheet src="jquery.dataTables.css"/>


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
      border-left: 1px solid darken(#f1f3f5, 10%);
    }

    ::-webkit-scrollbar-thumb {
      background: darken(#f1f3f5, 20%);
    }


</style>
</head>

<body>
<content tag="nav">
%{--<li class="dropdown">
    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Application Status <span class="caret"></span></a>
    <ul class="dropdown-menu">
        <li><a href="#">Environment: ${grails.util.Environment.current.name}</a></li>
        <li><a href="#">App profile: ${grailsApplication.config.grails?.profile}</a></li>
        <li><a href="#">App version:
            <g:meta name="info.app.version"/></a>
        </li>
        <li role="separator" class="divider"></li>
        <li><a href="#">Grails version:
            <g:meta name="info.app.grailsVersion"/></a>
        </li>
        <li><a href="#">Groovy version: ${GroovySystem.getVersion()}</a></li>
        <li><a href="#">JVM version: ${System.getProperty('java.version')}</a></li>
        <li role="separator" class="divider"></li>
        <li><a href="#">Reloading active: ${grails.util.Environment.reloadingAgentEnabled}</a></li>
    </ul>
</li>--}%
%{--<li class="dropdown">
    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Artefacts <span class="caret"></span></a>
    <ul class="dropdown-menu">
        <li><a href="#">Controllers: ${grailsApplication.controllerClasses.size()}</a></li>
        <li><a href="#">Domains: ${grailsApplication.domainClasses.size()}</a></li>
        <li><a href="#">Services: ${grailsApplication.serviceClasses.size()}</a></li>
        <li><a href="#">Tag Libraries: ${grailsApplication.tagLibClasses.size()}</a></li>
    </ul>
</li>--}%

  <sec:ifAnyGranted roles="ROLE_NewUser,ROLE_User,ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        %{--<li><a href="#"><g:link controller="asyncProject" action="index" ></g:link></a></li>--}%
        %{--<g:link controller="asyncProject" action="index" >Async Projects</g:link>--}%
        <g:link controller="analysisServer" action="index">Analysis Servers</g:link>
        <g:link controller="module" action="index">Moduls List</g:link>
        %{--<g:link controller="experiment" action="index" >Experiments</g:link>--}%

        %{--<li><a href="#">Domains: ${grailsApplication.domainClasses.size()}</a></li>--}%
        %{--<li><a href="#">Services: ${grailsApplication.serviceClasses.size()}</a></li>--}%
        %{--<li><a href="#">Tag Libraries: ${grailsApplication.tagLibClasses.size()}</a></li>--}%
      </ul>
    </li>
  </sec:ifAnyGranted>
%{--<sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
        <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"><i class="fa fa-gear"></i> Settings <span class="caret"></span></a>
        <ul class="dropdown-menu">
            --}%%{--<g:each var="plugin" in="${applicationContext.getBean('pluginManager').allPlugins}">
                <li><a href="#">${plugin.name} - ${plugin.version}</a></li>
            </g:each>--}%%{--
            <sec:ifAnyGranted roles="ROLE_Administrator">
                --}%%{--<g:link controller="user">Manage Users</g:link>--}%%{--
            </sec:ifAnyGranted>
            <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator">
                --}%%{--<g:link controller="user" action="newUsers">New Users</g:link>--}%%{--
            </sec:ifAnyGranted>
        </ul>
    </li>
</sec:ifAnyGranted>--}%
%{--
<sec:ifLoggedIn>
    -- <li><a><sec:username /></a></li> --
    -- <li><g:link controller="logout">Logout</g:link></li> --
    <p class="navbar-text" style="color: white">Logged in as: <sec:username /></p>
    <g:link controller="logout" class="navbar-text">Logout</g:link>
</sec:ifLoggedIn>
<sec:ifNotLoggedIn>
    <li><g:link controller='login' action='auth'>Login</g:link></li>
</sec:ifNotLoggedIn>
--}%
</content>
%{--<a href="#list-analysis" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%

<div class="nav" role="navigation">
  %{--<ul>--}%
    %{--<li><a class="home" href="${createLink(uri: '/experiment/index?eId=' + params?.eId)}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><g:link class="create" action="create" params="[eId: params?.eId]"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
    %{--<li>JUST FOR TEST..<g:link class="btn btn-primary" action="downloadResult" params="[eId: params?.eId]">downLoadResults</g:link></li>--}%
  %{--</ul>--}%
</div>

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

      <div id="analysisListTabl">
        <g:render template="templates/analysisListTbl" model="[]"/>
      </div>
      %{--TODO reactivate after analysis list tests--}%
      <div id="chkScript">
        %{--joblist=${jobList}--}%
        <g:render template="templates/statusCheckTempl" model="[jobList: jobList]"/>
      </div>
      <br/>
      <a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: params?.eId])}">
        <button class="btn btn-primary"><span class="glyphicon glyphicon-plus"></span>&nbspNew Analysis</button>
      </a>
      <button class="btn btn-info" onclick="checkTimer()"><span class="glyphicon glyphicon-refresh"></span>&nbsp;Update task status</button> <label style="margin-left: 5px" id="lastUpdatedTime"></label>
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
%{--<div class="modal fade in" id="resultModal" role="dialog">--}%
  %{--<div class="modal-dialog">--}%
    %{--<div class="modal-content">--}%
      %{--<div class="modal-body custom-height-modal">--}%
        %{--<div class="loading" id="loadingMessage" style="position:absolute; top: 50%; left: 50%">loading...</div>--}%
        %{--<iframe class="iframe-placeholder" src="" id="resultsFrame" name="resultsFrame" width="100%" height="100%" frameborder="0"></iframe>--}%
      %{--</div>--}%
    %{--</div>--}%
  %{--</div>--}%
%{--</div>--}%

%{--<div class="modal fade" tabindex="-1" id="aux${moduleParam?.id}" style="width:90%;" role="dialog">--}%
%{--
<div class="modal fade" tabindex="-1" id="resultModal" aria-labelledby="myModalLabel" aria-hidden="true" role="dialog">
  <div class="modal-dialog modal-lg">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <div id="myModalLabel">
          <h4 class="modal-title" style="text-align: left;">${moduleParam?.pLabel ? moduleParam?.pLabel : moduleParam?.pKey}</h4>
        </div>
      </div>

      <div class="modal-body custom-height-modal">
        <div class="loading" id="loadingMessage" style="position:absolute; top: 50%; left: 50%">loading...</div>
        --}%
%{--<object style="width: 100%;height:800px;" data="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}" ></object>--}%%{--

        <object style="width: 100%;height:100%;" data="${g.createLink(controller: 'analysis', action: 'downloadResultReport', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile])}" ></object>
        --}%
%{--
          <iframe src="/assets/gating/index.html" style="width: 90%; height: 300px" scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0"></iframe>
        --}%%{--

      </div>

      <div class="modal-footer">
        <button type="button" id="aux${moduleParam?.id}SaveBtn" style="display: none" class="left btn btn-success" >Save</button>
        <button type="button" id="aux${moduleParam?.id}CloseBtn" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>
--}%



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
    /*
    TODO reactivate this code after analysis list test
    setTimeout( function(){
        $("#msg").fadeOut('fast');
    }, 2500);
    */

//  var intrvalTmr = setInterval(checkTimer, 3000);
//  setInterval(checkTimer(), 3000);

    /*$("iframe#resultsFrame").load(function () {
      $("#loadingMessage").hide();
    });
*/
  /*  $("object#resultsFrame").load(function () {
      $("#loadingMessage").hide();
    });
*/
  });
</script>
</body>
</html>