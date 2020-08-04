<%@ page import="flowgate.Module; flowgate.ExpFile; flowgate.Experiment" %>
<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
  <style>
  .fade {
    display: none;
  }

  .fade.in {
    display: block;
  }
  </style>
</head>

<body>

<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.title}">Experiment</a>
    <a href="${g.createLink(controller: 'analysis', action: 'index', params: [eId: experiment?.id])}" class="breadcrumb dark">Analysis Tasks</a>
    <a href="#!" class="breadcrumb dark">Create New Analysis</a>
  </div>
</div>

<h2><g:message code="analysis.create.label" default="Create New Analysis"/></h2>

<g:hasErrors bean="${this.analysis}">
  <g:eachError var="err" bean="${this.analysis}">
    <script>
      document.addEventListener('DOMContentLoaded', function () {
        M.toast({
          html: '<span><g:message error="${err}"/></span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
          displayLength: Infinity,
          classes: 'red'
        });
      });
    </script>
  </g:eachError>
</g:hasErrors>

<g:set var="sss" bean="springSecurityService"/>

<g:if test="${dsCount == 0}">
  <p>There is no dataset defined in this experiment. You need to <a
      href="${createLink(controller: 'dataset', action: 'create', params: [eId: experiment?.id])}">create a dataset</a> first.</p>
</g:if>
<g:else>
  <g:form controller="analysis" action="save" enctype="multipart/form-data" onsubmit="enablePreloadOverlay()">
    <g:hiddenField name="user" value="${sss?.currentUser?.id}"/>
    <g:hiddenField name="analysisStatus" value="${1}"/>
    <g:hiddenField name="eId" value="${params?.eId}"/>
    <g:hiddenField name="timestamp" value="${new Date().format('yyyy-MM-dd hh:mm:ss')}"/>

    <f:with bean="analysis">
      <ul class="tabs">
        <li class="tab col s2"><a class="active" href="#basic">Basic</a></li>
        <li class="tab col s2"><a href="#advanced">Advanced</a></li>
      </ul>

      <div class="row">
        <div class="input-field col s12">
          <g:select id="module" required="" name="module.id" from="${Module.list()}"
                    optionValue="${{ module -> "${module.label ? module.label : module.title}" }}"
                    optionKey="id" noSelection="${['': 'Select a Pipeline']}" onchange="moduleChange(this.value);"/>
          <label><g:message code="analysis.module.label" default="Analysis Pipeline"/> *</label>
        </div>

        <div class="input-field col s12">
          <input type="text" name="analysisName" id="analysisName" required>
          <label for="analysisName"><g:message code="analysis.name.label" default="Task Name"/> *</label>
        </div>

        <div class="input-field col s12">
          <textarea name="analysisDescription" id="analysisDescription" class="materialize-textarea"></textarea>
          <label for="analysisDescription">Description</label>
        </div>

        <div id="modParams">
          <g:render template="templates/moduleParams" model="[module: module]"/>
        </div>

        <div class="input-field col s12">
          <button type="submit" class="btn waves-effect waves-light">${message(code: 'analysis.create.btn.label', default: 'Submit')}</button>
          <a href="${createLink(controller: 'analysis', action: 'index', params: [eId: eId])}" class="btn-flat">Return to Analysis List</a>
        </div>
      </div>

      <script>
        function moduleChange(moduleId) {
          var _data = {eId: ${params?.eId}, modId: moduleId <g:if test="${params?.dsId != null && !params?.dsId.isEmpty()}">, dsId: ${params?.dsId}</g:if>}
          $.ajax({
            url: "${createLink(controller: 'analysis', action: 'axModulChange')}",
            dataType: "json",
            type: "get",
            data: _data,
            success: function (data) {
              $("#modParams").html(data.modParams);
              $('#infoBoxModal').hide();

              var modalElems = document.querySelectorAll('.modal');
              M.Modal.init(modalElems);

              var selectElems = document.querySelectorAll('select');
              var instances = M.FormSelect.init(selectElems);

              var tooltipElems = document.querySelectorAll('.tooltipped');
              M.Tooltip.init(tooltipElems);
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
    </f:with>
  </g:form>

  <g:isOwnerOrRoles object="experiment" objectId="${params.eId}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
    <div class="fixed-action-btn">
      <a href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: params.eId])}" class="btn-floating btn-large waves-effect waves-light tooltipped"
         data-tooltip="Manage Datasets" data-position="left">
        <i class="material-icons">settings</i>
      </a>
    </div>
  </g:isOwnerOrRoles>
</g:else>
<script>
  document.addEventListener('DOMContentLoaded', function () {
    var tabElems = document.querySelectorAll('.tabs');
    M.Tabs.init(tabElems);

    var selectElems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(selectElems);

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });

  function enablePreloadOverlay() {
    $('.preload-background').css('display', 'flex');
  }
</script>
</body>
</html>
