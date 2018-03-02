<div class="pull-right">
    <span data-toggle="tooltip" title="${session?.projCardView ? 'show as list' : 'show as cards'}" >
        <div class="btn btn-default" onclick="toggleProjView()"><i class="fa ${session?.projCardView ? 'fa-list' : 'fa-th' }"></i></div>
    </span>
</div>
<h3 class="text-center">Projects</h3>
<br />
<div class="bootcards-list">
    <div class="panel panel-default">
        <div class="panel-body">
            <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick">
            <form>
                <div class="row">
                    <div class="col-xs-11">
                        <div class="form-horizontal form-group">
                            <input id="filterInput" type="text" class="form-control" onchange="setFilter()" placeholder="Search Projects..." value="${session?.filterString}">
                            <i class="fa fa-search"></i>
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
            <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick">
                <br/>
                %{--<br/>--}%
            </g:isNotAffilOrRoles>
        </div>
        <div class="list-group">
            <g:each var="project" in="${projectList}">
                <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_ProjectClick">
                    %{--<a class="list-group-item noLinkBlack" href="${createLink(controller: 'project', action: 'index', params:[pId: project?.id])}">--}%
                    <a class="list-group-item noLinkBlack ${session?.searchLst?.find {it == project?.id} != null ? 'findSel' : ''}" href="/flowgate/project/index?pId=${project?.id}">
                        <i class="fa fa-3x fa-file-text-o img-rounded pull-left" ></i>
                        <h4 class="list-group-item-heading">${project?.title}</h4>
                        <p class="list-group-item-text">${project?.description}</p>
                    </a>
                </g:isAffilOrRoles>
                <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_ProjectClick">
                    <div class="list-group-item ${session?.searchLst?.find {it == project?.id} != null ? 'findSel' : ''}">
                        <i class="fa fa-3x fa-file-text-o img-rounded pull-left" style=""></i>
                        <h4 class="list-group-item-heading">${project?.title}</h4>
                        <p class="list-group-item-text">${project?.description}</p>
                    </div>
                </g:isNotAffilOrRoles>
            </g:each>
        </div>
    </div>
</div>
