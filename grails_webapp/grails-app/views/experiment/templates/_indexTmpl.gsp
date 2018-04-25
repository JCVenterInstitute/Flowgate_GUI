<%@ page import="flowgate.ExpFile; flowgate.Experiment" %>
<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${experiment?.id == session?.experimentOpenId?.toLong()}">
  <div class="row">
  <h1 class="page-header">
    <g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
      <input class="form-control" name="title" value="${experiment?.title}" onchange="experimentTitleInputBlur(${experiment?.id}, this.value)" style="width: 50%;">
    </g:if>
    <g:else>${experiment?.title}</g:else>
  </h1>
  </div>

  <g:if test="${experiment?.id == session.experimentEditModeId?.toLong()}">
    <textarea id="experimentDescription" class="form-control" rows="10" cols="130" onchange="experimentDescriptionInputBlur(${experiment?.id}, this.value)"
              onfocus="this.select()">${experiment?.description}</textarea>
  </g:if>
  <g:else>
    <textarea class="form-control" readonly="true" rows="10" cols="130">${experiment?.description}</textarea>
  </g:else>

  <div id="bottomBtnBar">
    <g:render template="/experiment/templates/indexBottomBtnBar" model="[experiment: experiment]"/>
  </div>

  <div class="row">
    <div class="col-sm-12">
      %{--TODO disable collapseAll or expandAll if all are already collapsed or expanded--}%
      %{--<g:link controller="expFile" action="collapseAll" params="[eId: experiment?.id]" >--}%
      <div class="btn btn-info" onclick="expFileCollapseAllClick(${experiment?.id})">collapse all</div>
      %{--</g:link>--}%
      %{--<g:link controller="expFile" action="expandAll" params="[eId: experiment?.id]" >--}%
      <div class="btn btn-info" onclick="expFileExpandAllClick(${experiment?.id})">expand all</div>
    %{--</g:link>--}%
    %{--<g:if test="${experiment?.id == session.experimentEditModeId?.toLong()}">--}%
    %{--<sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_AddFcs">--}%
      <sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
        <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">
          <div class="pull-right btn btn-info" style="cursor: pointer">
            <i class="fa fa-plus"></i>&nbsp;Add/Upload FCS
          </div>
        </a>
      </sec:ifAnyGranted>
      <sec:ifNotGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
        <g:if test="${utilsService.isAffil('experiment', experiment?.id)}">
        %{--<div class="pull-right btn btn-info" onclick="btnAddExpFileClick(${experiment?.id})"><i class="glyphicon glyphicon-plus"></i> Add/Upload File</div>--}%
          <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">
            <div class="pull-right btn btn-info" style="cursor: pointer">
              <i class="fa fa-plus"></i>&nbsp;Add/Upload FCS
            </div>
          </a>
        </g:if>
        <g:else>
          <br/><br/>
        </g:else>
      </sec:ifNotGranted>
    </div>
  </div>
  <br/>

%{--${experiment?.expFiles.dump()}--}%
%{--<app-el-box *ngIf="experiment" [experiment]="experiment"></app-el-box>--}%
%{--TODO move to controller--}%
%{--<g:each var="expFile" in="${experiment?.expFiles}">--}%
  <g:each var="expFile" in="${ExpFile.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true)}">
    <div id="expFile-${expFile?.id}">
      <g:render template="/expFile/expFileTmpl" model="[experiment: experiment, expFile: expFile]"/>
    </div>
  </g:each>
</g:if>
<g:render template="/shared/manageUsers" model="[objectType: 'Experiment', object: experiment]"/>
