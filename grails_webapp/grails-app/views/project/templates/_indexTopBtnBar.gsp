<div class="fixed-action-btn" id="add-experiment-action-btn">
  <a class="btn-floating btn-large waves-effect waves-light tooltipped" href="${createLink(controller: 'experiment', action: 'create', params: [pId: project?.id])}"
     data-tooltip="Create a new experiment" data-position="left">
    <i class="material-icons">add</i>
  </a>
</div>

<g:isOwnerOrRoles object="project" objectId="${project?.id}" roles="ROLE_Administrator,ROLE_Admin">
  <div class="fixed-action-btn">
    <a class="btn-floating click-to-toggle btn-large">
      <i class="large material-icons">menu</i>
    </a>
    <ul>
      <li><a class="btn-floating tooltipped" href="${createLink(controller: 'project', action: 'edit', params: [id: project?.id])}"
             data-tooltip="Edit Project" data-position="top">
        <i class="material-icons">edit</i>
      </a></li>
      <li><a class="btn-floating modal-trigger tooltipped" href="#delete-project-modal"
             data-tooltip="Delete Project" data-position="top">
        <i class="material-icons">delete</i>
      </a></li>
      <li><a class="btn-floating modal-trigger tooltipped" href="#erase-project-modal"
             data-tooltip="Erase Project" data-position="top">
        <i class="material-icons">delete_forever</i>
      </a></li>
    </ul>
  </div>
  <script>
    $(document).ready(function () {
      $("#add-experiment-action-btn").css("bottom", "93px");
    });
  </script>
</g:isOwnerOrRoles>


<div id="delete-project-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to delete project</h4>
    <h6>Title</h6>

    <p>${project?.title}</p>
    <h6>Description</h6>

    <p>${project?.description}</p>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <a href="${createLink(controller: 'project', action: 'delete', params: [id: project?.id])}" class="modal-close waves-effect waves-green btn-flat">Confirm</a>
  </div>
</div>

<div id="erase-project-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to erase project</h4>
    <h6>Title</h6>

    <p>${project?.title}</p>
    <h6>Description</h6>

    <p>${project?.description}</p>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <a href="${createLink(controller: 'project', action: 'erase', params: [id: project?.id])}" class="modal-close waves-effect waves-green btn-flat">Confirm</a>
  </div>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var actionElems = document.querySelectorAll('.fixed-action-btn');
    M.FloatingActionButton.init(actionElems, {direction: 'left', hoverEnabled: false});

    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems);

    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });
</script>
