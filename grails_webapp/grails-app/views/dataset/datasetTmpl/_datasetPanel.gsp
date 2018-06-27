<%@ page import="flowgate.Dataset" %>

<div class="form-horizontal">
  <div class="form-group">
    <label class="col-sm-2 control-label">Subset Name:</label>

    <div id="datasetField" class="col-sm-4">
      <g:render template="datasetTmpl/datasetFieldEdit" model="[experiment: experiment, dsId: ds.id]"/>
    </div>

    <div class="col-sm-2">
      <button type="button" class="btn btn-warning" onclick="editDs(${ds.id});"><i class="fa fa-pencil"></i>Edit Subset Name</button>
    </div>

    <div class="col-sm-4">
      <button type="button" class="btn btn-primary" onclick="addDs(${ds.id});"><i class="fa fa-plus"></i>Add New Subset</button>
      <a href="/flowgate/experiment/index?eId=${experiment?.id}" class="btn btn-primary">Return to Experiment</a>
    </div>
  </div>
</div>
