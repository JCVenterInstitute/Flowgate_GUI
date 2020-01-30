<div class="col s12 m6 l6 valign-wrapper">
  <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
    <div class="input-field col s12 m6 l6 h2-right">
      <i class="material-icons prefix" style="line-height: 2.96;">search</i>
      <input type="text" id="filterInput" placeholder="Search in projects" onchange="setFilter()" value="${filterString}">
    </div>
  </g:isAffilOrRoles>

%{--<sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ProjectShowActive">
  <span style="font-size: x-small">
    <g:checkBox name="showInactive" value="${session?.showInactive ? true : false}"
                onclick="document.location=\'${createLink(controller: 'project', action: 'toggleShowinctive')}\'"/>&nbsp;Show Inactive&nbsp;&nbsp;
  </span>
</sec:ifAnyGranted>--}%
</div>

<g:each var="project" in="${projectList}">
  <div class="col s12">
    <div class="card">
      <div class="card-content">
        <span class="card-title">${project?.title}</span>

        <p>${project?.description}</p>
      </div>

      <div class="card-action">
        <a class="${session?.searchLst?.find { it == project?.id } != null ? 'findSel' : ''}"
           href="${createLink(controller: 'project', action: 'index', params: [pId: project?.id])}">Go to Project</a>
      </div>
    </div>
  </div>
</g:each>

<sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectCreate">
  <div class="fixed-action-btn">
    <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="${createLink(controller: 'project', action: 'create')}" data-tooltip="Create a new project" data-position="left">
      <i class="material-icons">add</i>
    </a>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var tooltipElems = document.querySelectorAll('.tooltipped');
      M.Tooltip.init(tooltipElems);
    });
  </script>
</sec:ifAnyGranted>
