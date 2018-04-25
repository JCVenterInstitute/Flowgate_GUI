<h1 class="page-header">Create Experiment</h1>
<g:form action="save" class="col-xs-6 col-sm-3">
  <g:hiddenField name="datasets" value="${null}"/>
  <g:hiddenField name="experimentMeta" value=""/>
  <g:hiddenField name="pId" value="${pId}"/>

  <div class="form-group">
    <label for="title">Title</label>
    <input type="text" class="form-control" id="title" name="title" placeholder="Title">
  </div>

  <div class="form-group">
    <label for="description">Description</label>
    <input type="text" class="form-control" id="description" name="description" placeholder="Description">
  </div>

  <div class="form-group">
    <label for="experimentHypothesis">Experiment Hypothesis</label>
    <input type="text" class="form-control" id="experimentHypothesis" name="experimentHypothesis" placeholder="Experiment Hypothesis">
  </div>
  <button type="submit" name="create" class="btn btn-primary">${message(code: 'default.button.create.label', default: 'Create')}</button>
  <a href="/flowgate/project/index?pId=${pId}" type="submit" name="back" class="btn btn-warning">Back</a>
</g:form>