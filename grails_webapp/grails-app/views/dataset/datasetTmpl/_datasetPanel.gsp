<%@ page import="flowgate.Dataset" %>
<style>
#datasetList {
  border: black 1px solid;
  padding: 10px 10px 10px 10px;
  overflow-y: scroll;
  overflow-x: hidden;
  max-height: 700px;
}
</style>

<div id="datasetField" >
  <g:render template="datasetTmpl/datasetFieldEdit" model="[experiment: experiment, dsId: params.dsId]" />
</div>
<hr/>
<div id="datasetList">
  <g:each in="${Dataset.findAllByExperiment(experiment)}" var="dataset" >
    <p><g:link controller="dataset" action="ds_edit" id="${experiment.id}" params="[dsMode:'edit', dsId: dataset.id]">${dataset.name}</g:link></p>
  </g:each>
</div>