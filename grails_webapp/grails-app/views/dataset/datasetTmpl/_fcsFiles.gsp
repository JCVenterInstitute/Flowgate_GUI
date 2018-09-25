<div id="fcsFiles">
  <g:set var="split" value="${Math.ceil(expFileCandidatesList?.size() / 3).toInteger()}"/>
  <g:set var="counter" value="${0}"/>
  <g:each in="${expFileCandidatesList?.sort { it.fileName }}" var="expFile" status="index">
    <g:set var="counter" value="${counter + 1}"/>
    <g:if test="${counter % split == 1}"><div class="col-sm-4"></g:if>
    <p><g:checkBox class="fcs_files" name="file_${expFile.id}" checked="${dataset.expFiles ? expFile.id in dataset.expFiles.id : expFile.id in dataset.expFiles }"/>&nbsp;<span>${expFile.fileName}</span></p>
    <g:if test="${counter % split == 0 || index + 1 == expFileCandidatesList?.size()}"></div></g:if>
  </g:each>
</div>

<script type="text/javascript">
  $(document).ready(function () {
    $("#fcsSelectAll").click(function () {
      $(".fcs_files").prop('checked', $(this).prop('checked'));
    });
    $('.fcs_files').change(function(){ //".checkbox" change
      //uncheck "select all", if one of the listed checkbox item is unchecked
      if(this.checked == false){ //if this item is unchecked
        $("#fcsSelectAll")[0].checked = false; //change "select all" checked status to false
      }

      //check "select all" if all checkbox items are checked
      if ($('.fcs_files:checked').length == $('.fcs_files').length ){
        $("#fcsSelectAll")[0].checked = true; //change "select all" checked status to true
      }
    });

    $('.s_h_filter').click(function(){
      $("#metaData").toggle(500)
      $("i", this).toggleClass("fa-angle-up fa-angle-down");
    });
  });
</script>