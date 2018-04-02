%{--<asset:javascript src="jquery-2.2.0.min.js"/>--}%
<style>
  #metaBox {
    /* margin-right: auto; */
    /* margin-top: 20px; */
    /* background-color: #eefaff; */
    /* box-shadow: #ee50e3 1px 0; */
    border: black 1px solid;
    padding: 10px 10px 10px 10px;
    overflow-y: scroll;
    overflow-x: hidden;
    max-height: 700px;
  }
  /* The switch - the box around the slider */
  .switch {
    position: relative;
    display: inline-block;
    /* width: 60px; */
    width: 20px;
    /* height: 34px; */
    height: 10px;
  }

  /* Hide default HTML checkbox */
  .switch input {display:none;}

  /* The slider */
  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
  }

  .slider:before {
    position: absolute;
    content: "";
    /* height: 26px; */
    height: 4px;
    /* width: 26px; */
    width: 4px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
  }

  input:checked + .slider {
    /* background-color: #2196F3; */
    background-color: #5cb85c;
  }

  input:focus + .slider {
    /* box-shadow: 0 0 1px #2196F3; */
    box-shadow: 0 0 1px #5cb85c;
  }

  input:checked + .slider:before {
    /* webkit-transform: translateX(26px); */
    -webkit-transform: translateX(10px);
    /* -ms-transform: translateX(26px); */
    -ms-transform: translateX(10px);
    /* transform: translateX(26px); */
    transform: translateX(10px);
  }

  /* Rounded sliders */
  .slider.round {
    /* border-radius: 34px; */
    border-radius: 4px;
    /* border: 1px black solid; */
  }

  .slider.round:before {
    border-radius: 50%;
  }
</style>
%{--<g:set var="experiment" value="${experiment}" />--}%

<p>&nbsp;</p>
%{--eF:${expFile}--}%
<p><strong>Filter</strong>
<!-- Rounded switch -->
<label class="switch pull-right" style="margin-right:20px;margin-top:10px">
    <g:checkBox type="checkbox" name="filterFiles" value="${session.filterFiles}" />
    %{--<g:link action="toggleFilter" id="${experiment.id}" ><span class="slider round" ></span></g:link>--}%
    %{--<span class="slider round" onclick="${createLinkTo(action: 'toggleFilter', id: experiment.id)}"></span>--}%
    <span class="slider round" onclick="setFilter();"></span>
    %{--<span class="slider round" onclick="${g.actionSubmit(action: 'toggleFilter', value: 'toggleFilter', params:[id: experiment.id])}"></span>--}%
</label>
</p>
<div id="metaBox">
  <g:each in="${experiment?.expMetadatas?.sort{it.mdCategory}?.mdCategory?.unique()}" var="catgy">
    <p>${catgy}</p>
    <g:each in="${experiment?.expMetadatas.findAll{it.mdCategory== catgy}?.sort{it.dispOrder} }" var="catGy">
      <p>&nbsp;&nbsp;&nbsp;&nbsp;<strong>${catGy.mdKey}</strong></p>
      <g:each in="${experiment?.expMetadatas.find{it.mdKey == catGy.mdKey}?.mdVals?.sort{it.dispOrder}}" var="mVals">
        %{--mVals=${mVals.id.toString()}--}%
        <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<g:checkBox name="mVal_${mVals.id}_chb" />&nbsp;&nbsp;${mVals.mdValue}</p>
      </g:each>
    </g:each>
    %{--<g:if test="${catNo}"--}%
  </g:each>
</div>
<script type="text/javascript">
  function setFilter(){
    var eId = ${experiment.id};
    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'toggleFilter')}",
      dataType: "json",
      data: {"id": JSON.stringify(eId)},
      type: "get",
      /* success: function (data) {
        console.log('toggleFilter success!');
      },
      error: function (request, status, error) {
        alert(error);
      },*/
      complete: function () {
        console.log('ajax completed');
      }
    });
   }
</script>
