<%@ page import="flowgate.Dataset" %>
<div class="row">
  <div class="btn btn-default" onclick="editDs(${ds.id});"><i class="fa fa-pencil" ></i>Edit Dataset</div>
  <div class="btn btn-default" onclick="addDs(${ds.id});"><i class="fa fa-plus" ></i>Add New Dataset</div>
</div>
<div class="row">
  <div id="datasetField" >
    <g:render template="datasetTmpl/datasetFieldEdit" model="[experiment: experiment, dsId: ds.id ]" />
  </div>
  %{--<div class="pull-right">
    <button type="submit" class="btn btn-success"  >Submit</button>
  </div>
  --}%
</div>
