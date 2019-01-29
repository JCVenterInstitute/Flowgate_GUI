<div class="bootcards-list">
  <div class="panel panel-default">
    %{--<div class="panel-body">
      <g:isAffilOrRoles roles="ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentClick,ROLE_ExperimentSearch">
        <form>
          <div class="col-xs-10">
            <div class="input-group">
              <input id="filterInput" type="text" class="form-control" style="position: initial" onchange="setFilter()" placeholder="Search Experiments..."
                     value="${session?.filterString}">
              <i class="fa fa-search"></i>
              <span class="input-group-btn">
                <button class="btn btn-primary" type="button" onclick="clearFilter()"><i class="fa fa-close"></i></button>
              </span>
            </div>
          </div>

          <div class="col-xs-2" style="text-align: right;">
            <span data-toggle="tooltip" title="${session?.projCardView ? 'show as list' : 'show as cards'}">
              <div class="btn btn-default" style="color: #333;" onclick="toggleExpView()"><i class="fa ${session?.expCardView ? 'fa-list' : 'fa-th'}"></i></div>
            </span>
          </div>
        </form>
      </g:isAffilOrRoles>
      <g:isNotAffilOrRoles roles="ROLE_ExperimentClick,ROLE_ExperimentSearch">
        <br/>
      </g:isNotAffilOrRoles>
    </div>--}%

    <div class="list-group">
      <g:each var="experiment" in="${experimentList}">
        <g:isAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClick,ROLE_ExperimentEdit">
          <a class="list-group-item noLinkBlack" href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}">
            <i class="fa fa-3x fa-file-text-o img-rounded pull-left" style=""></i>
            <h4 class="list-group-item-heading">${experiment?.title}</h4>

            <p class="list-group-item-text">${experiment?.description}</p>
          </a>
        </g:isAffilOrRoles>
        <g:isNotAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentClick,ROLE_ExperimentEdit">
          <div class="list-group-item">
            <i class="fa fa-3x fa-file-text-o img-rounded pull-left" style=""></i>
            <h4 class="list-group-item-heading">${experiment?.title}</h4>

            <p class="list-group-item-text">${experiment?.description}</p>
          </div>
        </g:isNotAffilOrRoles>
      </g:each>
    </div>
  </div>
  <g:if test="${experimentList == null || experimentList.isEmpty()}">
    There is no experiment for this project.
  </g:if>
</div>