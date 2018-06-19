<%@ page import="flowgate.Dataset; flowgate.ExperimentMetadataValue" %>
<style>
#fileBoxAssigned {
  border: black 1px solid;
  padding: 10px 10px 10px 10px;
  overflow-y: scroll;
  overflow-x: hidden;
  max-height: 635px;
}
</style>
<p>FCS files assigned to this subset:</p>
<div class="row">
  <div class="col-sm-12">
    <div class="pull-left btn btn-default" onclick="selAllAssFcs(${ds.id})" >Select All</div>
    <div class="pull-right btn btn-default" onclick="deselAllAssFcs(${ds.id})"  >Deselect All</div>
  </div>
</div>

<div id="fileBoxAssigned">
  <g:each in="${expFileAssignedList?.sort { it.fileName }}" var="expFile">
    <p><g:checkBox name="cbFcsFileAssId_${expFile.id}" checked="${expFile.id in session.fAssSels}"/>&nbsp;
      %{--<span id="" style="font-weight:${(expFile.id in ds?.expFiles*.id) ? 'bold' : 'normal'}">${expFile.fileName}--}%%{--&nbsp;${expFile.id}--}%%{--</span>--}%
      <span id="" >${expFile.fileName}%{--&nbsp;${expFile.id}--}%</span>
      %{--<g:if test="${expFile.id in ds?.expFiles*.id}">--}%
        %{--<i class="glyphicon glyphicon-ok"></i>--}%
      %{--</g:if>--}%
    </p>
  </g:each>
</div>
<br/>
<br/>
<sec:ifAnyGranted roles="ROLE_Tester">
  <div style="padding-left: 20px">
    <g:each in="${experiment.expFiles.sort { it.fileName }}" var="expFile">
      <g:each in="${expFile.metaDatas}" var="mVals">
        <p>${expFile.fileName} - ${mVals.mdVal}</p>
      </g:each>
    </g:each>
  </div>
</sec:ifAnyGranted>


