<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main" />
  <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}" />
  <title><g:message code="default.show.label" args="[entityName]" /></title>
</head>

<body>
%{--<a href="#show-analysis" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>--}%
<div class="nav" role="navigation">
  %{--<ul>--}%
    %{--<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>--}%
    %{--<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
    %{--<li><a class="home" href="${createLink(uri: '/experiment/index?eId='+ this.analysis.experiment.id)}"><g:message code="default.home.label"/></a></li>--}%
    %{--<li><g:link class="list" action="index"  params="[eId: this.analysis.experiment.id]" ><g:message code="default.list.label" args="[entityName]" /></g:link></li>--}%
    %{--<li><g:link class="create" action="create" params="[eId: this.analysis.experiment.id]"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
  %{--</ul>--}%
</div>
<g:if test="${flash?.resultMsg}">
  <br/>
  <br/>
  <br/>
  <div class="text-center">
    <div class="alert alert-success alert-dismissible" role="alert">
      <button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>
      <strong>Error in Job Result!</strong>&nbsp;&nbsp;${flash?.resultMsg}
    </div>
  </div>
</g:if>
<g:else>
    <div id="show-analysis" class="content scaffold-show" role="main">
    <h3 class="text-center"><g:message code="analysis.showResult.label" args="[entityName]" default="Analysis Results" /></h3>
    <g:if test="${flash.message}">
      <div class="message" role="status">${flash.message}</div>
    </g:if>
    <f:with bean="analysis">
      <div class="fieldcontain">
        <label for="exp">Experiment</label>
        %{--<div class="form-control">--}%
        <f:display id="exp" property="experiment.title" />
        %{--</div>--}%
      </div>

      <div class="fieldcontain">
        <label for="analysisName">Analysis Name</label>
        <f:display id="analysisName" property="analysisName" />
      </div>

      <div class="fieldcontain">
        <label for="analysisDescription">Analysis Description</label>
        <f:display id="analysisDescription" property="analysisDescription" />
      </div>

      <div class="fieldcontain">
        <label for="analysisStatus">Analysis Status</label>
        <span id="analysisStatus">
          %{--<i class=" fa fa-circle" style="color: ${analysis.analysisStatus == 1 ? 'lightgrey' : analysis.analysisStatus == 2 ? 'orange': analysis.analysisStatus == 3? 'lawngreen' : analysis.analysisStatus == -1 ? 'red' : 'brown'}"></i>--}%
          ${analysis.analysisStatus == 1 ? 'init' : analysis.analysisStatus == 2 ? 'pending': analysis.analysisStatus == 3 ? 'results ready' : analysis.analysisStatus == -1 ? 'error' : 'done'}
        </span>
      </div>

      <div class="fieldcontain">
        <label for="jobNumber">Job Number</label>
        <span id="jobNumber">${analysis.jobNumber.toString()}</span>
      </div>

      <div class="fieldcontain">
        <label for="usr">Sent by</label>
        <f:display id="usr" property="user.username" />
      </div>

      <div class="fieldcontain">
        <label for="tstmp">Date Created</label>
        <span><g:formatDate id="tstmp" date="${analysis.timestamp}" format="MM/dd/yyyy hh:mm:ss"/></span>
      </div>

      <div class="fieldcontain">
        <label for="jobComplete">Date Completed</label>
        <g:if test="${jobResult.dateCompleted}">
          <span><g:formatDate id="jobComplete" date="${new Date().parse("yyyy-MM-dd'T'hh:mm:ss",jobResult?.dateCompleted)}" format="MM/dd/yyyy hh:mm:ss"/></span>
        </g:if>
      </div>

      <div class="fieldcontain">
        <label for="jobStatusMsg">Job Complete Status</label>
        <span id="jobStatusMsg">
          <i class=" fa fa-circle" style="color: ${!jobResult?.status?.hasError ? 'lawngreen' : 'red'}"></i>
          ${jobResult?.status?.statusMessage}
        </span>
      </div>
    </f:with>
    <br/>
    <br/>
    %{--${jobResult.dump()}--}%
    <br/>
    <div class="row" style="max-width: 100%">
      <div class="col-sm-12">
        <g:render template="results/resultsGrid" />
      </div>
    </div>
    %{--TODO remove for normal user / change visibility    --}%
    <g:form resource="${this.analysis}" method="DELETE">
      <fieldset class="text-center">
        <g:link class="edit btn btn-primary" action="edit" resource="${this.analysis}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
        <input class="delete btn btn-danger" type="submit" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
      </fieldset>
    </g:form>
  </div>
</g:else>
</body>
</html>
