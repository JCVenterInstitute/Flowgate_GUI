<%@ page import="flowgate.Module; flowgate.ExpFile; flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
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

<div class="container">
  <h1 class="page-header"><g:message code="analysis.create.label" default="Add New Analysis"/></h1>

%{--Errormessage in layout --}%
%{--<g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
</g:if>
<g:hasErrors bean="${this.analysis}">
<ul class="errors" role="alert">
    <g:eachError bean="${this.analysis}" var="error">
    <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
    </g:eachError>
</ul>
</g:hasErrors>--}%
  <g:set var="sss" bean="springSecurityService"/>
  %{--<g:form action="save" enctype="multipart/form-data" useToken="true">--}%
  <g:form controller="analysis" action="save" enctype="multipart/form-data">
    <div class="form-horizontal">
      %{--<f:all bean="analysis"/>--}%
      %{--<g:hiddenField name="datasets" value="${null}" />--}%
      %{--<g:hiddenField name="protocol" value="${null}" />--}%
      %{--<g:hiddenField name="dataInputFile" value="inputFile1" />--}%
      <g:hiddenField name="user" value="${sss?.currentUser?.id}"/>
      <g:hiddenField name="analysisStatus" value="${1}"/>
      %{--<g:hiddenField name="analysisIdchar" value="0" />--}%
      <g:hiddenField name="eId" value="${params?.eId}"/>
      <g:hiddenField name="timestamp" value="${new Date().format('yyyy-MM-dd hh:mm:ss')}"/>
      <f:with bean="analysis">
        <div class="panel with-nav-tabs panel-default">
          <ul class="nav nav-tabs tabs-2" role="tablist" id="paramsTabs">
            <li class="nav-item active">
              <a class="nav-link" data-toggle="tab" href="#basic">Basic</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" data-toggle="tab" href="#advanced">Advanced</a>
            </li>
          </ul>

          <div class="tab-content m-5">
            <div class="form-group">
              <label for="module" class="col-sm-2 control-label">Analysis Module<span class="required-indicator">*</span></label>
              <div class="col-sm-10">
                <g:select id="module" required="" class="form-control" name="module.id" from="${Module.list()}" optionValue="title" optionKey="id" noSelection="${['': 'Select a module']}"
                          onchange="moduleChange(this.value);"/>
              </div>
            </div>
            <div class="form-group">
              <label for="analysisName" class="col-sm-2 control-label">Analysis Name<span class="required-indicator">*</span></label>
              <div class="col-sm-10">
                <input type="text" name="analysisName" id="analysisName" class="form-control" >
              </div>
            </div>
            <div class="form-group">
              <label for="analysisDescription" class="col-sm-2 control-label">Analysis Name<span class="required-indicator">*</span></label>
              <div class="col-sm-10">
                <textarea type="text" name="analysisDescription" id="analysisDescription" class="form-control" rows="2" cols="290" maxlength="150"></textarea>
              </div>
            </div>
            <div id="modParams">
              <g:render template="templates/moduleParams" model="[module: module]"/>
            </div>
            <div class="form-group">
              <div class="col-sm-offset-2 col-sm-10">
                <g:submitButton name="create" class="save btn btn-primary" value="${message(code: 'analysis.create.btn.label', default: 'Run/Submit')}"/>
                <a href="/flowgate/analysis/index?eId=${eId}" class="btn btn-warning"><i class="fa fa-times"></i>Cancel</a>
              </div>
            </div>

            <script>
              function moduleChange(moduleId) {
                $.ajax({
                  url: "${createLink(controller: 'analysis', action: 'axModulChange')}",
                  dataType: "json",
                  type: "get",
                  data: {eId: ${params?.eId}, modId: moduleId},
                  success: function (data) {
                    $("#modParams").html(data.modParams);
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

          </div>
        </div>
      </f:with>
      %{--<h3 class="col-sm-offset-1">Pipleline Parameters</h3>
      <br />
      <div class="row">
        <div class="col-xs-2 right">
          Filter Step&nbsp;<span style="color:red">*</span>
        </div>
        <div class="col-xs-2">
          <g:radioGroup name="filterStep" values="[0,1]" labels="['No','Yes']" value="0">
            <g:message code="${it.label}" />&nbsp;${it.radio}&nbsp;&nbsp;
          </g:radioGroup>
        </div>
      </div>
      <div class="row">
        <div class="col-xs-2 right">
          Parameters in FLOCK Step&nbsp;<span style="color:red">*</span>
        </div>
        <div class="col-xs-2">
          <g:radioGroup name="flockStep" values="[0,1]" labels="['Auto','Manual']" value="1">
            <g:message code="${it.label}" />&nbsp;${it.radio}&nbsp;&nbsp;
          </g:radioGroup>
        </div>
      </div>--}%
      %{--<hr />--}%
      %{--<div class="btn btn-default" onclick="selAllClick()" >Select All</div>--}%
      %{--<div class="btn btn-default" onclick="unselAllClick()" >Deselect All</div>--}%
      %{--<br />--}%
      %{--<br />--}%
      %{--<div id="expFileLst">--}%
      %{-- -- <g:render template="templates/expFilesSelect" model="[eId: params?.eId, expFileSelected: ExpFile.findAllByExperimentAndIsActive(Experiment.get(params?.eId?.toLong()), true)*.id]" /> -- --}%
      %{--<g:render template="templates/expFilesSelect" model="[eId: params?.eId, expFileSelected: []]" />--}%
      %{--</div>--}%
    </div>
  </g:form>
</div>
<script type="text/javascript">
  function selAllClick() {
    $.ajax({
      url: "${createLink(controller: 'analysis', action: 'axSelectAllFcs')}",
      dataType: "json",
      type: "get",
      data: {eId: ${params?.eId}},
      success: function (data) {
        $("#expFileLst").html(data.contentList);
      },
      error: function (request, status, error) {
        alert(error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function unselAllClick() {
    $.ajax({
      url: "${createLink(controller: 'analysis', action: 'axUnselectAllFcs')}",
      dataType: "json",
      type: "get",
      data: {eId: ${params?.eId}},
      success: function (data) {
        $("#expFileLst").html(data.contentList);
      },
      error: function (request, status, error) {
        alert(error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }
</script>
</body>
</html>
