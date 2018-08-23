<g:hiddenField name="dsMode" value="${dsMode}" />
%{--<g:hiddenField name="dsId" value="${ds?.id}" />--}%
%{--<g:hiddenField name="eId" value="${experiment?.id}" />--}%
<input class="form-control" id="dsName" name="name" placeholder="dataset name" value="${dsMode=='dsEdit' ? ds.name : ''}"/>
<script>
  $(document).ready(function () {
    $("#datasetName").focus();
  });
</script>
