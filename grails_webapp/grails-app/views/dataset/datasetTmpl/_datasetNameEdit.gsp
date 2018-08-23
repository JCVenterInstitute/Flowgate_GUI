<%@ page import="flowgate.Dataset" %>
%{--dsId ${dsId} pdsId ${params.dsId} ds:${ds}--}%
<g:hiddenField name="dsMode" value="${dsMode}" />
<g:hiddenField name="dsId" value="${dsId}" />
%{--<g:hiddenField name="eId" value="${experiment?.id}" />--}%

<g:if test="${Dataset.findAllByExperiment(experiment).size()>0}">
  <g:if test="${dsMode == 'dsEdit'}">
    <g:textField id="dsName" name="name" value="${ds?.name}" />
  </g:if>
  <g:else>
    <g:select class="form-control" name="selDsId" from="${Dataset.findAllByExperiment(experiment)}" optionValue="name" optionKey="id"
              value="${dsId ?: Dataset.findAllByExperiment(experiment) ? Dataset.findAllByExperiment(experiment)?.first() : null }"
              onChange="dsSelChange(this.value);"
      />
    <g:hiddenField name="name" value="${ds?.name}" />
  </g:else>
</g:if>
<g:else>
   <g:textField id="dsName" name="name" value="new dataset" />
</g:else>
%{--<script>--}%
  %{--$(document).ready(function () {--}%
    %{--$("#datasetName").focus();--}%
  %{--});--}%
%{--</script>--}%
