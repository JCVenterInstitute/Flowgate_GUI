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
  <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_NewUsersList">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        <li><g:link controller="analysisServer" action="index">Analysis Servers</g:link></li>
        <li><g:link controller="module" action="index">Moduls List</g:link></li>
      </ul>
    </li>
  </sec:ifAnyGranted>
</content>

<div class="container">
  <h1 class="page-header"><g:message code="analysis.create.label" default="Add New Analysis"/></h1>
  <g:set var="sss" bean="springSecurityService"/>
  <g:form controller="analysis" action="save" enctype="multipart/form-data">
    <div class="form-horizontal">
      <div class="col-sm-offset-11">
        <g:link url="https://google.com" ><div class="btn btn-primary">Jupyter Link</div></g:link>
      </div>
      <br/>
      <g:hiddenField name="user" value="${sss?.currentUser?.id}"/>
      <g:hiddenField name="analysisStatus" value="${1}"/>
      <g:hiddenField name="eId" value="${params?.eId}"/>
      <g:hiddenField name="timestamp" value="${new Date().format('yyyy-MM-dd hh:mm:ss')}"/>
      <f:with bean="analysis">
        <div class="panel with-nav-tabs panel-default">
          <ul class="nav nav-tabs tabs-3" role="tablist" id="paramsTabs">
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
                <g:select id="module" required="" class="form-control" name="module.id" from="${Module.list()}" optionValue="title"
                          optionKey="id" noSelection="${['': 'Select a module']}" onchange="moduleChange(this.value);"/>
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
    </div>
  </g:form>
</div>
</body>
</html>
