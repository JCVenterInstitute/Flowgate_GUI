<%@ page import="flowgate.Dataset" %>
<g:select name="ds.id" from="${Dataset.findAllByExperiment(experiment)}" optionValue="name" optionKey="id"
          value="${dsId ?: Dataset.findAllByExperiment(experiment).first()}"
          onChange="dsSelChange(this.value);"
  />
