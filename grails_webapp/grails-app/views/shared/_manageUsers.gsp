%{--<%@ page import="flowgate.ExperimentUser; flowgate.User" %>--}%
<div class="modal fade in" id="manage${objectType}UsersModal-${object?.id}" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <g:if test="${objectType == 'Experiment'}">
        <g:render template="/shared/mUserFormExperiment" model="[experiment: object]"/>
      </g:if>
      <g:if test="${objectType == 'Project'}">
        <g:render template="/shared/mUserFormProject" model="[project: object]"/>
      </g:if>
      <g:if test="${objectType == 'ProjExp'}">
        <g:render template="/shared/mUserFormProjExp" model="[experiment: object]"/>
      </g:if>
    </div>
  </div>
</div>