<g:hiddenField name="formMode" value="${dsMode}" />
<g:hiddenField name="dsId" value="${dataset?.id}" />
<g:hiddenField name="eId" value="${experiment?.id}" />
<input class="form-control" id="datasetName" name="name" placeholder="dataset name" value="${dsMode=='dsEdit' ? dataset.name : ''}"/>
<script>
  $(document).ready(function () {
    $("#datasetName").focus();
  });
</script>
