<%@ page import="flowgate.Experiment; flowgate.Analysis" %>
%{--<div class="row" style="padding: 0;">--}%
%{--<div class="row">--}%
%{--<div class="pull-right btn btn-group">--}%
%{--<div class="btn btn-toolbar">--}%
%{--
<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
    <div class="btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">
        <i class="glyphicon glyphicon-pencil"></i>&nbsp;edit
    </div>
</g:isOwnerOrRoles>
--}%

%{--
<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClone">
    <div class="btn btn-primary" onclick="experimentCloneClick(${experiment?.id})" >
        <i class="fa fa-clone"></i>&nbsp;&nbsp;Clone Experiment
    </div>
</g:isOwnerOrRoles>
--}%

%{--
<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentUploadTemplate">
    <div class="btn btn-primary" onclick="experimentUploadTemplateClick(${experiment?.id})" >
        <i class="fa fa-upload"></i>&nbsp;&nbsp;Upload FCS Template
    </div>
</g:isOwnerOrRoles>
--}%

%{--
<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
    <a class="btn btn-info" href="/experiment/delete?id=${experiment?.id}"
        onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" >
        <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
    </a>
</g:isOwnerOrRoles>
--}%

%{--<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentManageUsers">--}%
%{--<span data-toggle="tooltip" title="analysis" >--}%
%{--
<div style="cursor: pointer" data-toggle="modal" data-target="#createAnalysisModal-${experiment?.id}">
    <div class="btn btn-default" >
        <i class="fa fa-pencil-square-o"></i>
    </div>
</div>
--}%
%{--</span>--}%
%{--</g:isOwnerOrRoles>--}%

%{--<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentManageUsers">--}%
%{--<span data-toggle="tooltip" title="manage users" >--}%
%{--
<div style="cursor: pointer" data-toggle="modal" data-target="#manageExperimentUsersModal-${experiment?.id}">
    <div class="btn btn-default" >
        <i class="fa fa-user"></i>
    </div>
</div>
--}%
%{--</span>--}%
%{--</g:isOwnerOrRoles>--}%
%{--</div>--}%
%{--</div>--}%

<div class="row mt-4 mb-4">
  <div class="col-sm-12">
  %{-- <g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
       <div class="btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">done</div>
   </g:if>
   <g:else>--}%
    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
      <span data-toggle="tooltip" title="Analysis">
        <g:if test="${experiment.analyses.size() > 0}">
          <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'index', params: [eId: experiment?.id])}" >
        </g:if>
        <g:else>
          <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: experiment?.id])}" >
        </g:else>
        <div class="btn btn-default" style="cursor: pointer">
          <i class="fa fa-bar-chart">&nbsp;
            <g:if test="${experiment?.analyses?.size() > 0}">
            %{--<span style="color: red">--}%
              <span class="label label-pill label-default">${experiment?.analyses?.size()}</span>
            %{--</span>--}%
            </g:if>
          </i>
          Analysis
        </div>
      </a>
      </span>
    </g:isOwnerOrRoles>


    %{--<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">--}%
    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
      <span data-toggle="tooltip" title="FCS File Annotation">
        <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment?.id)}" >
        <div class="btn btn-default" style="cursor: pointer">
          <i class="fa fa-adn fa-lg"></i>FCS File Annotation
        </div>
      </a>
      </span>
    </g:isOwnerOrRoles>


    <g:if env="development">
      <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
        <span data-toggle="tooltip" title="Manage Datasets orig">
          <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'dataset', action: 'ds_edit', params: [id: experiment?.id, eId: experiment?.id])}" >
            <div class="btn btn-default" style="cursor: pointer">
              <i class="fa fa-database"></i>Manage Datasets orig
            </div>
          </a>
        </span>
      </g:isOwnerOrRoles>
    </g:if>

    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
      <span data-toggle="tooltip" title="Manage Datasets new">
        <a class="noLinkBlack " style="background-color: transparent" href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" >
          <div class="btn btn-default" style="cursor: pointer">
            <i class="fa fa-database"></i>Manage Datasets
          </div>
        </a>
      </span>
    </g:isOwnerOrRoles>

    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit,ROLE_ExperimentManageUsers">
      <span data-toggle="tooltip" title="Manage Users">
        %{--
        <div class="btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">
            <i class="glyphicon glyphicon-pencil"></i>&nbsp;edit
        </div>
        --}%
        %{--<div style="cursor: pointer" data-toggle="modal" data-target="#manageExperimentUsersModal-${experiment?.id}">--}%
        <div style="cursor: pointer" class="btn btn-default " data-toggle="modal" data-target="#manageExperimentUsersModal-${experiment?.id}">
          <i class="fa fa-user"></i>Manage Users
        </div>
        %{--</div>--}%
      </span>
    </g:isOwnerOrRoles>

  %{--<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
  <span data-toggle="tooltip" title="Notifications" >
      <div  style="cursor: pointer" class="btn btn-default " data-toggle="modal" data-target="#experimentNotificationsModal">
          <i class="fa fa-bell"></i>
      </div>
  </span>
  </g:isOwnerOrRoles>--}%

  %{--
  <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClone">
      <div class="btn btn-primary" onclick="experimentCloneClick(${experiment?.id})" >
          <i class="fa fa-clone"></i>&nbsp;&nbsp;Clone Experiment
      </div>
  </g:isOwnerOrRoles>
  --}%

  %{--    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentUploadTemplate">
      <div class="btn btn-primary" onclick="experimentUploadTemplateClick(${experiment?.id})" >
          <i class="fa fa-upload"></i>&nbsp;&nbsp;Upload FCS Template
      </div>
  </g:isOwnerOrRoles>
  --}%

  %{--<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
  --}%%{--
  <g:link class="btn btn-info" controller="experiment" action="delete" resource="${experiment}"
          onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" >
      <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
  </g:link>
  --}%%{--
      <a class="btn btn-info" href="/experiment/delete?id=${experiment?.id}"
         onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" >
          <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
      </a>
  </g:isOwnerOrRoles>
--}%

  %{-- moved to bottom bar --}%
  %{-- <div class="btn-toolbar btn btn-default" data-toggle="tooltip" title="manage users" > --}%
  %{--
  <div class="btn-toolbar btn btn-default"  data-toggle="modal" data-target="#manageExperimentUsersModal">
      --}%%{--<button type="button" class="" data-toggle="modal" data-target="#manageUsersModal-${expFile?.id}"><i class="fa fa-users"></i></button>--}%%{--
      <i class="fa fa-user"></i>
  </div>
  --}%
  %{--</g:else>--}%
  </div>
</div>