<div class="pull-right">
    <span data-toggle="tooltip" title="${session?.projCardView ? 'show as list' : 'show as cards'}" >
        <div class="btn btn-default" onclick="toggleProjView()"><i class="fa ${session?.projCardView ? 'fa-list' : 'fa-th' }"></i></div>
    </span>
</div>
<h3 class="text-center">Projects</h3>
<br />
<div class="bootcards-summary">
    <div class="panel panel-default">
        <div class="panel-body">
            <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
            <form>
                <div class="row">
                    <div class="col-xs-11">
                        <div class="form-horizontal form-group">
                            <input id="filterInput" type="text" class="form-control" onchange="setFilter()" placeholder="Search Projects..." value="${session?.filterString}">
                            %{--<i class="fa fa-search"></i>--}%
                        </div>
                    </div>
                    <div class="col-xs-1">
                        <div class="btn btn-primary btn-block" onclick="clearFilter()">
                            <i class="fa fa-close"></i>
                        </div>
                    </div>
                </div>
            </form>
            </g:isAffilOrRoles>
            <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
                <br/>
            </g:isNotAffilOrRoles>
        </div>
        <div class="row">
            <div class="bootcards-cards">
            <g:each var="project" in="${projectList}">
                <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
                    <div class="col-xs-6 col-sm-3">
                        isAffilorRole
                        <g:render template="templates/projCardULockedTmpl" model="[project: project]" />
                    </div>
                </g:isAffilOrRoles>
                <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
                    <div class="col-xs-6 col-sm-3">
                        not affilorrole
                        <g:render template="templates/projCardLockedTmpl" model="[project: project]" />
                    </div>
                </g:isNotAffilOrRoles>
            </g:each>
            </div>
        </div>
    </div>
</div>
