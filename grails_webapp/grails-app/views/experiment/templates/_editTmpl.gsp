<div id="edit-experiment" class="content scaffold-edit" role="main">
  <h1><g:message code="default.edit.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
  <g:hasErrors bean="${this.experiment}">
    <div class="row m-0">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
        <g:eachError bean="${this.experiment}" var="error">
          <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
        </g:eachError>
      </div>
    </div>
  </g:hasErrors>
  <g:form resource="${this.experiment}" method="PUT" class="col-xs-12 col-sm-6">
    <g:hiddenField name="version" value="${this.experiment?.version}"/>
    <div class="form-group">
      <label for="title">Title *</label>
      <textarea class="form-control" id="title" name="title" placeholder="Title" rows="5" required>${experiment.title}</textarea>
    </div>
    <div class="form-group">
      <label for="description">Description *</label>
      <textarea class="form-control" id="description" name="description" placeholder="Description" rows="10" required>${experiment.description}</textarea>
    </div>
    <div class="form-group">
      <label for="experimentMeta">Experiment Meta</label>
      <input type="text" class="form-control" id="experimentMeta" name="experimentMeta" placeholder="Experiment Meta" value="${experiment.experimentMeta}">
    </div>
    <div class="form-group">
      <label for="experimentMeta">Experiment Hypothesis</label>
      <input type="text" class="form-control" id="experimentHypothesis" name="experimentHypothesis" placeholder="Experiment Hypothesis" value="${experiment.experimentHypothesis}">
    </div>

    <button type="submit" class="btn btn-primary">${message(code: 'default.button.update.label', default: 'Update')}</button>
    <a type="submit" name="back" class="btn btn-warning" href="${createLink(controller: 'experiment', action: 'index', params: [eId: experiment?.id])}">Back</a>
  </g:form>
</div>