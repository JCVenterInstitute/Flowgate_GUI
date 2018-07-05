
<div class="pull-right">
  <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ProjectEdit">
  %{--
  <g:link class="btn btn-primary edit noLinkBlack" controller="project" action="edit" id="${project?.id}">
      <i class="glyphicon glyphicon-edit"></i>&nbsp;&nbsp;Edit Project
  </g:link>
  --}%
    <a class="btn btn-primary edit noLinkBlack" href="${createLink(controller: 'project', action: 'edit', params: [id: project?.id])}">
      <i class="glyphicon glyphicon-edit"></i>&nbsp;&nbsp;Edit Project
    </a>
  </sec:ifAnyGranted>

  <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ProjectCreate">
  %{--
  <g:link class="btn btn-primary create noLinkBlack" controller="project" action="create">
      <i class="glyphicon glyphicon-plus"></i>&nbsp;&nbsp;Add Project
  </g:link>
  --}%
    <a class="btn btn-primary create noLinkBlack" href="${createLink(controller: 'project', action: 'create')}">
      <i class="glyphicon glyphicon-plus"></i>&nbsp;&nbsp;Add Project
    </a>
  </sec:ifAnyGranted>


  <g:if test="${actionName == 'index'}">
    <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ProjectCreate">
      <div class="btn btn-primary" onclick="projectCloneClick(${project?.id})">
        <i class="fa fa-clone"></i>&nbsp;&nbsp;Clone Project
      </div>
    </sec:ifAnyGranted>
  </g:if>

  <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ProjectDelete">
  %{--
  <g:link class="btn btn-primary noLinkBlack" controller="project" action="delete" id="${project?.id}"
          onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');">
      <i class="glyphicon glyphicon-trash"></i>&nbsp;&nbsp;Delete Project
  </g:link>
  --}%
    <a class="btn btn-primary noLinkBlack" href="${createLink(controller: 'project', action: 'delete', params: [id: project?.id])}"
       onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');">
      <i class="glyphicon glyphicon-trash"></i>&nbsp;&nbsp;Delete Project
    </a>
  </sec:ifAnyGranted>

  <sec:ifAnyGranted roles="ROLE_SuperAdministrator,ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_ProjectErase">
  %{--
  <g:link class="btn btn-primary noLinkBlack" controller="project" action="erase" id="${project?.id}"
          onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');">
      <i class="fa fa-eraser"></i>&nbsp;&nbsp;Erase Project
  </g:link>
  --}%
    <a class="btn btn-primary noLinkBlack" href="${createLink(controller: 'project', action: 'erase', params: [id: project?.id])}"
       onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');">
      <i class="fa fa-eraser"></i>&nbsp;&nbsp;Erase Project
    </a>
  </sec:ifAnyGranted>
</div>