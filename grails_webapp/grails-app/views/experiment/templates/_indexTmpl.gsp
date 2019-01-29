<%@ page import="flowgate.ExpFile; flowgate.Experiment" %>
<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${flash.message}">
  <div class="row justify-content-center ">
    <div class="alert alert-info text-center" role="alert">${flash.message}</div>
  </div>
</g:if>
<g:form name="updateExperiment" method="PUT" controller="experiment" action="update" id="${experiment?.id}">
<g:if test="${experiment?.id == session?.experimentOpenId?.toLong()}">
  <g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
    <div class="form-group">
      <label for="title">Experiment Title</label>
      <input class="form-control" id="title" name="title" value="${experiment?.title}" style="width: 75%;">
    </div>
  </g:if>
  <g:else><h1 class="page-header">${experiment?.title}</h1></g:else>

  <g:if test="${experiment?.id == session.experimentEditModeId?.toLong()}">
    <div class="form-group">
      <label for="title">Experiment Description</label>
      <textarea id="experimentDescription" name="description" class="form-control" rows="10" cols="130" onfocus="this.select()">${experiment?.description}</textarea>
    </div>
  </g:if>
  <g:else>
    <textarea class="form-control" readonly="true" rows="10" cols="130">${experiment?.description}</textarea>
  </g:else>

  <div id="bottomBtnBar">
    <g:render template="/experiment/templates/indexBottomBtnBar" model="[experiment: experiment]"/>
  </div>

  <div class="row">
    <div class="col-sm-12">
      %{-- TODO disable collapseAll or expandAll if all are already collapsed or expanded --}%
      %{-- <g:link controller="expFile" action="collapseAll" params="[eId: experiment?.id]" > --}%
      %{-- <div class="btn btn-info" onclick="expFileCollapseAllClick(${experiment?.id})">collapse all</div> --}%
      %{-- </g:link> --}%
      %{-- <g:link controller="expFile" action="expandAll" params="[eId: experiment?.id]" > --}%
      %{-- <div class="btn btn-info" onclick="expFileExpandAllClick(${experiment?.id})">expand all</div> --}%
      %{-- </g:link> --}%
    %{--<g:if test="${experiment?.id == session.experimentEditModeId?.toLong()}"> --}%
    %{--<sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_AddFcs">--}%
    </div>
  </div>
  <br/>

%{-- ${experiment?.expFiles.dump()} --}%
%{-- <app-el-box *ngIf="experiment" [experiment]="experiment"></app-el-box> --}%
%{-- TODO move to controller --}%
%{-- <g:each var="expFile" in="${experiment?.expFiles}"> --}%
  <g:if test="${experiment}" >
    <g:if test="${experiment && ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment?.id?.toLong(), true), true)}" >
      This experiment currently contains <b>${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true).size()}</b> FCS files.
    %{--
    <g:each in="${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true)}" var="expFile" >
      <div id="expFile-${expFile?.id}">
        <g:render template="/expFile/expFileTmpl" model="[experiment: experiment, expFile: expFile]"/>
      </div>
    </g:each>
    --}%
    </g:if>
    <g:else>
      This experiment doesn't have any FCS file.
    </g:else>
  </g:if>

%{--
--}%
</g:if>
<g:render template="/shared/manageUsers" model="[objectType: 'Experiment', object: experiment]"/>
</g:form>