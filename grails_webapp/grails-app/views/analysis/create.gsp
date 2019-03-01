<%@ page import="flowgate.Module; flowgate.ExpFile; flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
  <style>
  .fade {
    display:none;
  }
  .fade.in {
    display:block;
  }


   /*modal full screen  */


    /*.modal {
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
      !*
      height: auto;
      min-height: 100%;
      *!
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
*/

  </style>
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
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
    <li><a href="${createLink(controller: 'analysis', action: 'index', params: [eId: experiment?.id])}">List of Analysis Task</a></li>
    <li class="active">Create New Analysis</li>
  </ul>
  <h1 class="page-header"><g:message code="analysis.create.label" default="Create New Analysis"/></h1>
  <g:hasErrors bean="${this.analysis}">
    <div class="row m-0">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
        <g:eachError bean="${this.analysis}" var="error">
          <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
        </g:eachError>
      </div>
    </div>
  </g:hasErrors>
  <g:set var="sss" bean="springSecurityService"/>
  <g:if test="${dsCount == 0}">
    <div class="row m-0">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
        There is no dataset defined in this experiment. You need to <a href="${createLink(controller: 'dataset', action: 'create', params: [eId: experiment?.id])}">create a dataset</a> first.
      </div>
    </div>
  </g:if>
  <g:else>
    <g:form controller="analysis" action="save" enctype="multipart/form-data">
      <div class="form-horizontal">
      %{--<div class="col-sm-offset-11">
        <g:link url="https://google.com" ><div class="btn btn-primary">Jupyter Link</div></g:link>
      </div>
      <br/>--}%
        <g:hiddenField name="user" value="${sss?.currentUser?.id}"/>
        <g:hiddenField name="analysisStatus" value="${1}"/>
        <g:hiddenField name="eId" value="${params?.eId}"/>
        <g:hiddenField name="timestamp" value="${new Date().format('yyyy-MM-dd hh:mm:ss')}"/>
        %{-- TODO remove after testing --}%
        <input id="acs" style="display: none;width: 1200px;" name="acsVal" value="['APC_CD14','SSC_A','pop6','AF700_CD3','PE_Cy7_CD19','pop7','AF700_CD3','PE_CD56','pop8','APC_eF780_CD4','BV650_CD8a','pop12','APC_eF780_CD4','FITC_CD25','pop14']" />

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
                <label for="module" class="col-sm-2 control-label"><g:message code="analysis.module.label" default="Analysis Pipeline" /><span class="required-indicator">*</span></label>

                <div class="col-sm-10">
                  <g:select id="module" required="" class="form-control" name="module.id" from="${Module.list()}" optionValue="${{ module -> "${module.label ? module.label : module.title}" }}"
                            optionKey="id" noSelection="${['': 'Select a Pipeline']}" onchange="moduleChange(this.value);"/>
                </div>
              </div>

              <div class="form-group">
                <label for="analysisName" class="col-sm-2 control-label"><g:message code="analysis.name.label" default="Task Name" /><span class="required-indicator">*</span></label>

                <div class="col-sm-10">
                  <input type="text" name="analysisName" id="analysisName" class="form-control" required>
                </div>
              </div>

              <div class="form-group">
                <label for="analysisDescription" class="col-sm-2 control-label"><g:message code="analysis.description.label" default="Task Description" /></label>

                <div class="col-sm-10">
                  <textarea type="text" name="analysisDescription" id="analysisDescription" class="form-control" rows="2" cols="290" maxlength="250"></textarea>
                </div>
              </div>


              <div id="modParams">
                <g:render template="templates/moduleParams" model="[module: module]"/>
              </div>

              <div class="form-group">
                <div class="col-sm-offset-2 col-sm-10">
                  <g:submitButton name="create" class="save btn btn-primary" value="${message(code: 'analysis.create.btn.label', default: 'Submit')}"/>
                  <a href="${createLink(controller: 'analysis', action: 'index', params: [eId: eId])}" class="btn btn-warning"><i class="fa fa-times"></i>Cancel</a>
                </div>
              </div>

              <script>
                function moduleChange(moduleId) {
                  var _data = {eId: ${params?.eId}, modId: moduleId <g:if test="${params?.dsId != null && !params?.dsId.isEmpty()}">, dsId: ${params?.dsId}</g:if> }
                  $.ajax({
                    url: "${createLink(controller: 'analysis', action: 'axModulChange')}",
                    dataType: "json",
                    type: "get",
                    data: _data,
                    success: function (data) {
                      $("#modParams").html(data.modParams);
                      $('#infoBoxModal').hide();
                      // $('#aux20').hide();
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
  </g:else>
</div>
</body>
</html>
