<div class="row" style="padding: 0;">
    <div class="pull-right">
        <g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
            <div class="btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">done</div>
        </g:if>
        <g:else>
            <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
                <div class="btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">
                    <i class="glyphicon glyphicon-pencil"></i>&nbsp;edit
                </div>
            </g:isOwnerOrRoles>

            <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClone">
                <div class="btn btn-primary" onclick="experimentCloneClick(${experiment?.id})" >
                    <i class="fa fa-clone"></i>&nbsp;&nbsp;Clone Experiment
                </div>
            </g:isOwnerOrRoles>

            <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentUploadTemplate">
                <div class="btn btn-primary" onclick="experimentUploadTemplateClick(${experiment?.id})" >
                    <i class="fa fa-upload" ></i>&nbsp;&nbsp;Upload FCS Template
                </div>
            </g:isOwnerOrRoles>

            <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
                %{--
                <g:link class="btn btn-info" controller="experiment" action="delete" resource="${experiment}"
                        onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" >
                    <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
                </g:link>
                --}%
                <a class="btn btn-info" href="/flowgate/experiment/delete?id=${experiment?.id}"
                    onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" >
                    <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
                </a>
            </g:isOwnerOrRoles>

            %{-- moved to bottom bar --}%
            %{-- <div class="btn-toolbar btn btn-default" data-toggle="tooltip" title="manage users" > --}%
            %{--
            <div class="btn-toolbar btn btn-default"  data-toggle="modal" data-target="#manageExperimentUsersModal-${experiment?.id}">
                --}%%{--<button type="button" class="" data-toggle="modal" data-target="#manageUsersModal-${expFile?.id}"><i class="fa fa-users"></i></button>--}%%{--
                <i class="fa fa-user"></i>
            </div>
            --}%
        </g:else>
    </div>
</div>