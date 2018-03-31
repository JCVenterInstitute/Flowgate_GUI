<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}"/>
  <title><g:message code="default.project.list.label" args="[entityName]" default="FlowGate - Projects"/></title>
</head>

<body>
<g:render template="/shared/nav"/>
<content tag="topBtnBar">
  <div class="row">
    <div id="topBtnBar" >
      <g:render template="templates/indexTopBtnBar" model="[project: this.project]" />
    </div>
    <g:render template="/shared/errorsMsgs" model="[bean: this.project]" />
  </div>
</content>

%{--TODO remove after testing, just for testing purpose!!!!--}%
<sec:ifAnyGranted roles="ROLE_Acs">
  <div class="nav" role="navigation">
    <ul>
      <li><g:link class="btn btn-default" controller="analysis" action="create" params="[eId: 1]"><g:message code="create.analysis.label" default="Create Analysis"/></g:link></li>
    </ul>
    <ul>
      <li><g:link class="btn btn-default" controller="analysis" action="index" params="[eId: 1]"><g:message code="analysis.index.label" default="Analyses"/></g:link></li>
    </ul>
    %{--
    <ul>
        <li><g:link class="btn btn-default" controller="analysisPipeline" action="create" params="[eId: 1]" ><g:message code="create.analysis.label" default="Create A Pipeline" /></g:link></li>
    </ul>
    --}%
    <ul>
      <li><g:link class="btn btn-default" controller="expFile" action="expFileCreate" params="[eId: 1]"><g:message code="create.expFile.label" default="Upload FCS"/></g:link></li>
    </ul>
  </div>
</sec:ifAnyGranted>

<div class="container">
  <h1 class="page-header">Projects</h1>
  <div id="pageContent">
    <g:if test="${session?.projCardView}">
      <g:render template="templates/projCardsTmpl"/>
    </g:if>
    <g:else>
      <g:render template="templates/projListTmpl"/>
    </g:else>
  </div>
</div>
<script>
  $('[data-toggle="tooltip"]').tooltip();

  function toggleProjView() {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axToggleView')}",
      dataType: 'json',
      type: "get",
      success: function (data) {
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function setFilter() {
    var filterString = document.getElementById("filterInput").value;
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axSearch')}",
      dataType: "json",
      type: "get",
      data: {filterString: filterString},
      success: function (data) {
        $("#pageContent").html(data.contentPage);
      },
//    error: function (request, status, error) {
      error: function (data) {
        alert(data);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function clearFilter() {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axClearSearch')}",
      dataType: "json",
      type: "get",
      success: function (data) {
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, message) {
        alert('Error! ' + message);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function clearFilter() {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axClearSearch')}",
      dataType: "json",
      type: "get",
      success: function (data) {
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, message) {
        alert('Error! ' + message);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

</script>
</body>
</html>