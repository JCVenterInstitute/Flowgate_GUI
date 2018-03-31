<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
  <asset:javascript src="jquery-2.2.0.min.js"/>
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
<a href="#list-analysis" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>

<div class="nav" role="navigation">
  <ul>
    %{--<li><a class="home" href="${createLink(uri: '/experiment/index?eId=' + params?.eId)}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><g:link class="create" action="create" params="[eId: params?.eId]"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
    %{--<li>JUST FOR TEST..<g:link class="btn btn-primary" action="downloadResult" params="[eId: params?.eId]">downLoadResults</g:link></li>--}%
  </ul>
</div>

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="list-analysis" class="content scaffold-list" role="main">
      <h1 class="page-header"><g:message code="default.list.label" args="[entityName]"/></h1>

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
<div class="modal fade in" id="resultModal" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-body custom-height-modal">
        <div class="loading" id="loadingMessage" style="position:absolute; top: 50%; left: 50%">loading...</div>
        <iframe class="iframe-placeholder" src="" id="resultsFrame" name="resultsFrame" width="100%" height="100%" frameborder="0"></iframe>
      </div>
    </div>
  </div>
</div>



<script>
  function showResult() {
    window.open("${g.createLink(controller: 'experiment', action: 'renderAnalysisHtml2')}", "resultsFrame")
  }

  $(document).ready(function () {

    /*
    TODO reactivate this code after analysis list test
    setTimeout( function(){
        $("#msg").fadeOut('fast');
    }, 2500);
    */

//                var intrvalTmr = setInterval(checkTimer, 3000);
//                setInterval(checkTimer(), 3000);

    $("iframe#resultsFrame").load(function () {
      $("#loadingMessage").hide();
    });

  });
</script>
</body>
</html>