<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
  %{--<asset:javascript src="jquery/jquery.dataTables.js"/>--}%
  %{--<asset:stylesheet src="jquery.dataTables.css"/>--}%
  <asset:javascript src="d3/d3.min.js"/>
</head>

<body>
<content tag="nav">

  <sec:ifAnyGranted roles="ROLE_NewUser,ROLE_User,ROLE_Admin,ROLE_Administrator">
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Settings <span class="caret"></span></a>
      <ul class="dropdown-menu">
        <g:link controller="analysisServer" action="index">Analysis Servers</g:link>
        <g:link controller="module" action="index">Moduls List</g:link>
      </ul>
    </li>
  </sec:ifAnyGranted>

<div class="nav" role="navigation"></div>

<div class="row">
  <div class="col-sm-offset-1 col-sm-10">
    <div id="list-analysis" class="content scaffold-list" role="main">
      <ul class="breadcrumb">
        <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" title="${experiment?.project?.title}">${experiment?.project?.title}</a></li>
        <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" title="${experiment?.title}">${experiment?.title}</a></li>
        <li class="active">List of Analysis Task</li>
      </ul>
      <h1 class="page-header">List of Analysis Task</h1>

      %{--<div id="analysisListTabl">--}%
        %{--<g:render template="templates/analysisListTbl" model="[]"/>--}%
      %{--</div>--}%
      %{--TODO reactivate after analysis list tests--}%
      %{--<div id="chkScript">--}%
        %{--joblist=${jobList}--}%
        %{--<g:render template="templates/statusCheckTempl" model="[jobList: jobList]"/>--}%
      %{--</div>--}%
      <br/>
      %{--<a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: params?.eId])}">--}%
        %{--<button class="btn btn-primary"><span class="glyphicon glyphicon-plus"></span>&nbspNew Analysis</button>--}%
      %{--</a>--}%
      %{--<sec:ifAnyGranted roles="ROLE_Tester,ROLE_Acs">--}%
        %{--<a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'heatM')}">--}%
          %{--<button class="btn btn-default">GenePattern Heatmap</button>--}%
        %{--</a>--}%
        %{--<a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'fcs2Csv')}">--}%
          %{--<button class="btn btn-default">GenePattern Fcs2Csv</button>--}%
        %{--</a>--}%
        %{--<a class="noLinkBlack text-center" style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'flock')}">--}%
          %{--<button class="btn btn-default">GenePattern flock</button>--}%
        %{--</a>--}%
      %{--  Buttons to test results page  --}%
        %{--<a href="javascript:void(0)" onclick="javascript:showResult()" width="200" height="200">--}%
          %{--<div class="btn btn-info">newHtml</div>--}%
        %{--</a>--}%

        %{--<div class="btn btn-info" onclick="javascript:showResult()" data-toggle="modal" data-target="#resultModal">newHtml2</div>--}%
      %{--</sec:ifAnyGranted>--}%

      <div class="days-hours-heatmap">
        <!-- calibration and render type controller -->
        <div class="calibration" role="calibration">
          <div class="group" role="example">
            <svg width="120" height="17">
            </svg>
            <div role="description" class="description">
              <label>Less</label>
              <label>More</label>
            </div>
          </div>
          <div role="toggleDisplay" class="display-control">
            <div>
              <input type="radio" name="displayType" checked/>
              <label>count</label>
            </div>
            <div>
              <input type="radio" name="displayType"/>
              <label>daily</label>
            </div>
          </div>
        </div>
        <!-- heatmap -->
        <svg role="heatmap" class="heatmap"></svg>
      </div>

      %{--<script type="text/javascript" src="hm.js"></script>--}%
      <asset:javascript src="hm.js"/>

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

<script>

  // $(document).ready(function () {
  //
  // });
</script>
</body>
</html>