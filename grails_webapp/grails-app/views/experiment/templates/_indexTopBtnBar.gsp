<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
  <div class="fixed-action-btn" style="bottom: 163px;">
    <g:if test="${experiment.analyses.size() > 0}">
      <a href="${g.createLink(controller: 'analysis', action: 'index', params: [eId: experiment?.id])}" class="btn-floating btn-large waves-effect waves-light tooltipped"
         data-tooltip="Show Analysis" data-position="left">
        <b>A</b>
      </a>
    </g:if>
    <g:else>
      <a href="${g.createLink(controller: 'analysis', action: 'create', params: [eId: experiment?.id])}" class="btn-floating btn-large waves-effect waves-light tooltipped"
         data-tooltip="Create a new analysis" data-position="left">
        <i class="material-icons">add</i>
      </a>
    </g:else>
  </div>
</g:isOwnerOrRoles>
<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
  <div class="fixed-action-btn" style="bottom: 93px;">
    <a href="${g.createLink(controller: 'dataset', action: 'index', params: [eId: experiment?.id])}" class="btn-floating btn-large waves-effect waves-light tooltipped"
       data-tooltip="Manage Datasets" data-position="left">
      <i class="material-icons">settings</i>
    </a>
  </div>
</g:isOwnerOrRoles>

<div class="fixed-action-btn">
  <a class="btn-floating click-to-toggle btn-large">
    <i class="large material-icons">menu</i>
  </a>
  <ul>
    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
      <li><a class="btn-floating tooltipped" href="${createLink(controller: 'experiment', action: 'edit', params: [id: experiment?.id])}"
             data-tooltip="Edit Experiment" data-position="top">
        <i class="material-icons">edit</i>
      </a></li>
    </g:isOwnerOrRoles>
    <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
      <li><a class="btn-floating modal-trigger tooltipped" href="#delete-experiment-modal" data-tooltip="Delete Experiment" data-position="top">
        <i class="material-icons">delete</i>
      </a></li>
    </g:isOwnerOrRoles>
  </ul>
</div>

<div id="delete-experiment-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to delete experiment</h4>
    <h6>Title</h6>

    <p>${experiment?.title}</p>
    <h6>Description</h6>

    <p>${experiment?.description}</p>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <a href="${createLink(controller: 'experiment', action: 'delete', params: [id: experiment?.id])}" class="modal-close waves-effect waves-green btn-flat">Confirm</a>
  </div>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var actionElems = document.querySelectorAll('.fixed-action-btn');
    M.FloatingActionButton.init(actionElems, {direction: 'left', hoverEnabled: false});

    var modalElems = document.querySelectorAll('#delete-experiment-modal');
    M.Modal.init(modalElems);

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>

%{--<g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
  <div class="btn btn-warning" onclick="toggleExpEditMode(${experiment?.id});">Cancel</div>
  <div class="btn btn-primary" onclick="toggleExpEditMode(${experiment?.id});document.getElementById('updateExperiment').submit();">Submit</div>
</g:if>--}%
