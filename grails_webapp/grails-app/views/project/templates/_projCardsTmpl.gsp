<div class="bootcards-summary bootcards-list">
  <div class="panel panel-default">
    <div class="panel-body">
      <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
        <form>
          <div class="col-xs-10">
            <div class="input-group">
              <input id="filterInput" type="text" class="form-control" style="position: initial" onchange="setFilter()" placeholder="Search Projects..."
                     value="${session?.filterString}">
              <i class="fa fa-search"></i>
              <span class="input-group-btn">
                <button class="btn btn-primary" type="button" onclick="clearFilter()"><i class="fa fa-close"></i></button>
              </span>
            </div>
          </div>

          <div class="col-xs-2" style="text-align: right;">
            <span data-toggle="tooltip" title="${session?.projCardView ? 'show as list' : 'show as cards'}">
              <div class="btn btn-default" style="color: #333;" onclick="toggleProjView()"><i class="fa ${session?.projCardView ? 'fa-list' : 'fa-th'}"></i></div>
            </span>
          </div>
        </form>
      </g:isAffilOrRoles>
      <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
        <br/>
      </g:isNotAffilOrRoles>
    </div>

    <div class="row" style="border: 1px solid #ddd;border-width: 1px 0;padding-top: 10px;margin-left: 20px;margin-right: 20px;">
      <g:each var="project" in="${projectList}">
        <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
          <div class="col-xs-6 col-sm-3">
            <g:render template="templates/projCardULockedTmpl" model="[project: project]"/>
          </div>
        </g:isAffilOrRoles>
        <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
          <div class="col-xs-6 col-sm-3">
            <g:render template="templates/projCardLockedTmpl" model="[project: project]"/>
          </div>
        </g:isNotAffilOrRoles>
      </g:each>
    </div>
  </div>
</div>
