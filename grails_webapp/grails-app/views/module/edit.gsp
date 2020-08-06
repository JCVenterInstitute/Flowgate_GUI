<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
  <style>
  .hiddendiv.common {
    display: none;
  }
  </style>
</head>

<body>

<h2 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h2>
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
<g:form resource="${this.module}" method="PUT" class="row">
  <g:hiddenField name="version" value="${this.module?.version}"/>
  <f:with bean="module">
    <div class="col s4">
      <div class="row">
        <f:field property="server" label="Server" required="true"/>
        <f:field property="label" label="Label" required="false"/>
        <f:field property="title" label="Title" required="true"/>
        <f:field property="name" label="Module or URN" required="true"/>
      </div>
    </div>

    <div class="col s8">
      <f:field property="descript" label="Module Description" required="false" rows="5"/>
    </div>
  </f:with>

  <g:if test="${module.moduleParams.size() > 0}">
    <div class="col s12">
      <h5>Module Parameters</h5>
      <f:table collection="${module.moduleParams}" properties="['pKey', 'pLabel', 'defaultVal', 'pBasic', 'pType', 'descr', 'exampleFile']"/>
    </div>
  </g:if>

  <div class="input-field col s12">
    <button class="btn waves-effect waves-light" type="submit">${message(code: 'default.button.update.label', default: 'Update')}</button>
    <a href="${createLink(controller: 'moduleParam', action: 'create', params: ['module.id': module.id])}" class="btn-flat">Add Module Parameter</a>
    <a class="btn-flat modal-trigger" href="#delete-module-modal">${message(code: 'default.button.delete.label', default: 'Delete')}</a>
    <g:link controller="module" action="index" class="btn-flat">Return to Module List</g:link>
  </div>
</g:form>

<div id="delete-module-modal" class="modal">
  <div class="modal-content">
    <h4>Confirm to delete module</h4>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <g:link action="delete" resource="${this.module}" class="modal-close waves-effect waves-light btn-flat">Confirm</g:link>
  </div>
</div>

<div class="fixed-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="javascript:fetchModuleParams();"
     data-tooltip="Search available parameters for this module" data-position="left">
    <i class="material-icons">search</i>
  </a>
</div>

<div id="module-params-list" class="modal bottom-sheet"></div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('descript').style.maxHeight = "300px";
    document.getElementById('descript').style.overflowY = "auto";

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);

    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems);
  });

  function fetchModuleParams() {
    $.ajax({
      url: "${createLink(controller: 'moduleParam', action: 'fetchModuleParamsForModule')}",
      dataType: "json",
      data: {id: ${this.module?.id}},
      type: "get",
      success: function (data, status, xhr) {
        if (data.success) {
          $("#module-params-list").html(data.modules);
          $('.modal').modal({
            onOpenStart: function () {
              var datasetValue = $("input[name='dataset']:checked").val();
              if(datasetValue && datasetValue != 'undefined') {
                $('#confirm-import').attr('href', $('#confirm-import').attr('href') + '?dataset=' + datasetValue);
              }
            }
          });
          $("#module-params-list").modal('open');
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
