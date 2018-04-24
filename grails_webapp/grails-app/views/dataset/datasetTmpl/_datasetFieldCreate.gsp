<g:hiddenField name="formMode" value="${dsMode}" />
<g:hiddenField name="dsId" value="${dataset.id}" />
<input class="form-control" name="dataset.name" placeholder="dataset name" value="${dsMode=='dsEdit' ? dataset.name : ''}"/>