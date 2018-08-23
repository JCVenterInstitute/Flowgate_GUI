<%@ page import="flowgate.Dataset" %>
%{--<g:hiddenField name="dsMode" value="${dsMode}" />--}%
%{--<g:hiddenField name="dsId" value="${ds?.id}" />--}%
%{--<g:hiddenField name="eId" value="${experiment?.id}" />--}%
%{--<g:hiddenField name="dsId" value="${dsId}" />--}%

<div class="form-horizontal">
  <div class="form-group">
    <label class="col-sm-2 control-label">Subset Name:</label>

    <div id="datasetNameField" class="col-sm-4">
      <g:render template="datasetTmpl/datasetNameEdit" model="[experiment: experiment, dsId: dsId, ds:ds]"/>
    </div>

    <div class="col-sm-2">
      <button type="button" class="btn btn-warning" onclick="editDs(${ds?.id});"><i class="fa fa-pencil"></i>Edit Subset Name</button>
    </div>

    <div class="col-sm-4">
      <button type="button" class="btn btn-primary" onclick="addDs(${ds?.id});"><i class="fa fa-plus"></i>Add New Subset</button>
      <a href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}" class="btn btn-primary">Return to Experiment</a>
    </div>
  </div>
</div>
