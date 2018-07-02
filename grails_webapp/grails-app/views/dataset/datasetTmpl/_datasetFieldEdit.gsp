<%@ page import="flowgate.Dataset" %>
<g:if test="${Dataset.findAllByExperiment(experiment).size()>0}">
  <g:select class="form-control" name="ds.id" from="${Dataset.findAllByExperiment(experiment)}" optionValue="name" optionKey="id"
            value="${dsId ?: Dataset.findAllByExperiment(experiment) ? Dataset.findAllByExperiment(experiment)?.first() : null }"
            onChange="dsSelChange(this.value);"
    />
</g:if>
<g:else>
   <g:textField name="dataset.name" value="new dataset" />
</g:else>
