<div class="pull-right">
    <div class="btn btn-default" onclick="toggleExpView()"><i class="fa ${session?.expCardView ? 'fa-list' : 'fa-th' }"></i></div>
</div>
<h3 class="text-center">Experiments</h3>
<br />
<div class="bootcards-list">
    <div class="panel panel-default">
        <div class="panel-body">
        <g:isAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ProjectClick,ROLE_ProjectSearch">
            <form>
                <div class="row" style="padding: 0;max-width: none;" >
                    <div class="col-xs-11">
                        <div class="form-horizontal form-group">
                            <input type="text" class="form-control" placeholder="Search Experiments...">
                            <i class="fa fa-search"></i>
                        </div>
                    </div>
                    <div class="col-xs-1">
                        <a class="btn btn-primary btn-block" href="#">
                            <i class="fa fa-close"></i>
                        </a>
                    </div>
                </div>
            </form>
        </g:isAffilOrRoles>
        <g:isNotAffilOrRoles object="project" objectId="${project?.id}" roles="ROLE_ProjectClick">
            <br/>
        </g:isNotAffilOrRoles>
        </div>
        <div class="row" style="padding: 0;max-width: none;">
            <div class="list-group">
                <g:each var="experiment" in="${experimentList}">
                    <g:isAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_ExperimentClick">
                        %{--<a class="list-group-item" href="${createLink(controller: 'experiment', action: 'index', params:[eId: experiment?.id])}">--}%
                        <a class="list-group-item noLinkBlack" href="/flowgate/experiment/index?eId=${experiment?.id}" >
                            <i class="fa fa-3x fa-file-text-o img-rounded pull-left" style=""></i>
                            <h4 class="list-group-item-heading">${experiment?.title}</h4>
                            <p class="list-group-item-text">${experiment?.description}</p>
                        </a>
                    </g:isAffilOrRoles>
                    <g:isNotAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_ExperimentClick">
                        <div class="list-group-item">
                            <i class="fa fa-3x fa-file-text-o img-rounded pull-left" style=""></i>
                            <h4 class="list-group-item-heading">${experiment?.title}</h4>
                            <p class="list-group-item-text">${experiment?.description}</p>
                        </div>
                    </g:isNotAffilOrRoles>
                </g:each>
            </div>
        </div>
    </div>
</div>