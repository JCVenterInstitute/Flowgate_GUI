<!DOCTYPE html>
<%@ page import="flowgate.AnalysisServer" defaultCodec="html" %>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.create.label" args="[entityName]"/></h2>
<g:hasErrors bean="${this.module}">
  <g:eachError var="err" bean="${this.module}">
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
<g:form action="save">
  <div class="row">
    <f:with bean="module">
      <div class="col s4">
        <div class="row">
          <div class="input-field col s12">
            <g:select name="server" from="${flowgate.AnalysisServer.list()}" required="" optionKey="id" optionValue="name"/>
            <label>Server*</label>
          </div>
          <f:field property="title" label="Title" required="true"/>
          <f:field property="label" label="Label"/>
          <f:field property="name" label="Module or URN" required="true"/>
        </div>
      </div>

      <div class="col s8">
        <div class="input-field">
          <label for="descript">Description</label>
          <g:textArea name="descript" id="descript" class="materialize-textarea" style="height: 300px"/>
        </div>
      </div>
    </f:with>

    <div class="input-field col s12">
      <button type="submit" class="btn waves-effect waves-light">${message(code: 'default.button.create.label', default: 'Create')}</button>
      <g:link controller="module" action="index" class="btn-flat">Return to Module List</g:link>
    </div>
  </div>
</g:form>


<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="javascript:fetchModules();"
     data-tooltip="Search available modules for selected server" data-position="left">
    <i class="material-icons">search</i>
  </a>
</div>

<div id="modules-list" class="modal bottom-sheet"></div>
<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
    document.getElementById('descript').style.maxHeight = "300px";
    document.getElementById('descript').style.overflowY = "auto";

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });

  function fetchModules() {
    var serverId = $("#server").val();
    $.ajax({
      url: "${createLink(controller: 'module', action: 'fetchModulesForServer')}",
      dataType: "json",
      data: {id: serverId},
      type: "get",
      success: function (data, status, xhr) {
        if (data.success) {
          $("#modules-list").html(data.modules);
          $('.modal').modal();
          $("#modules-list").modal('open');
          $('.tooltipped').tooltip();
        } else {
          M.toast({
            html: '<span>' + data.message + '</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
            displayLength: Infinity,
            classes: 'red'
          });
        }
      },
      error: function (request, status, error) {
        if (!error) error = "Error occured!"
        M.toast({
          html: '<span>' + error + '</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
          displayLength: Infinity,
          classes: 'red'
        });
      }
    });
  }
</script>
</body>
</html>
