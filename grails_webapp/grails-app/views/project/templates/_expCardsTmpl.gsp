<div class="pull-right">
    <div class="btn btn-default" onclick="toggleExpView()"><i class="fa ${session?.expCardView ? 'fa-list' : 'fa-th' }"></i></div>
</div>
<h3 class="text-center">Experiments</h3>
<br />
<div class="bootcards-summary">
    <div class="panel panel-default">
        <div class="panel-body">
            <g:isAffilOrRoles roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentClick,ROLE_ExperimentSearch">
                <form>
                    <div class="row" style="padding: 0;max-width: none;">
                    %{--<div class="row">--}%
                        <div class="col-xs-11">
                            <div class="form-horizontal form-group">
                                <input id="filterInput" type="text" class="form-control" onchange="setFilter()" placeholder="Search Experiments..." value="${session?.filterString}">
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
            <g:isNotAffilOrRoles roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentClick,ROLE_ExperimentSearch">
                <br/>
            </g:isNotAffilOrRoles>
        </div>
        <div class="row" style="padding: 0;max-width: none;">
            <div class="bootcards-cards">
                <g:each var="experiment" in="${experimentList}">
                    <div class="col-xs-6 col-sm-3">
                        <g:isAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClick,ROLE_ExperimentEdit">
                            <g:render template="templates/expCardULockedTmpl" model="[experiment: experiment]" />
                        </g:isAffilOrRoles>
                        <g:isNotAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClick,ROLE_ExperimentEdit">
                            <g:render template="templates/expCardLockedTmpl" model="[experiment: experiment]" />
                        </g:isNotAffilOrRoles>
                    </div>
                    <g:render template="/shared/manageUsers" model="[objectType: 'ProjExp', object: experiment]"/>
                </g:each>
            </div>
        </div>
    </div>
</div>
