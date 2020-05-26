<%@ page import="flowgate.Module" %>
<div class="modal-content">
  <h4>Available Parameters for ${module.title}</h4>
  <ul class="collection">
    <g:each in="${moduleParams}" var="params" status="i">
      <li class="collection-item avatar" id="params-${i}">
        <i class="material-icons circle">settings</i>
        <span class="title"><strong>${params.name}</strong></span>

        <p>${params.description} <br>
          ${params.TYPE}
        </p>

        <g:if test="${params.TYPE == 'FILE' || params.order >= 0}">
          <label class="secondary-content">
            <input name="dataset" type="radio" value="<g:if test="${params.name}">${params.name}</g:if><g:else>${params.label}</g:else>"/>
            <span>Mark as a dataset parameter</span>
          </label>
        </g:if>
      </li>
    </g:each>
  </ul>
</div>

<div class="modal-footer">
  <a href="#import-paramaters-modal" class="waves-effect waves-light btn-flat modal-trigger">Import</a>
  <a href="#!" class="modal-close waves-effect waves-green btn-flat">Close</a>
</div>

<div id="import-paramaters-modal" class="modal modal-fixed-footer">
  <div class="modal-content">
    <h4>Confirm to import module parameters for ${module.title}</h4>

    <p><strong>Note:</strong> This will remove current parameters if there is any.</p>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <a id="confirm-import" href="${createLink(controller: 'moduleParam', action: 'importParameters', params: [id: module?.id])}"
       class="modal-close waves-effect waves-green btn-flat">Confirm</a>
  </div>
</div>
