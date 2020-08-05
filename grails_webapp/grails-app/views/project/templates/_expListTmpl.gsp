<g:if test="${experimentList == null || experimentList.isEmpty()}">
  <p>There is no experiment for this project.</p>
</g:if>
<g:else>
  <div class="row">
    <g:each var="experiment" in="${experimentList}">
      <div class="col s12">
        <div class="card">
          <div class="card-content">
            <span class="card-title">${experiment?.title}</span>

            <p>${experiment?.description}</p>
          </div>

          <g:isAffilOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin">
            <div class="card-action">
              <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}">Go to Experiment</a>
            </div>
          </g:isAffilOrRoles>
        </div>
      </div>
    </g:each>
  </div>
</g:else>
