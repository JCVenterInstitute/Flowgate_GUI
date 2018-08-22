<%@ page import="flowgate.Dataset; flowgate.ExperimentMetadataValue" %>
<style>
#fileBoxCandidates {
  border: black 1px solid;
  padding: 10px 10px 10px 10px;
  overflow-y: scroll;
  overflow-x: hidden;
  max-height: 700px;
}
</style>
<p>FCS files in this experiment:</p>
<div class="row">
  <div class="col-sm-12">
    %{--<g:actionSubmit class="pull-left btn btn-default" action="selAllCandFcs" id="${experiment?.id}"
                    params="[pType: pType]" value="Select All" >
    </g:actionSubmit>
    <g:actionSubmit class="pull-right btn btn-default" action="deselAllCandFcs" id="${experiment?.id}"
                    params="[pType: pType]" value="Deselect All" >
    </g:actionSubmit>--}%
    <div class="pull-left btn btn-default" onclick="selAllCandFcs(${ds.id});" >Select All</div>
    <div class="pull-right btn btn-default" onclick="deselAllCandFcs(${ds.id});"  >Deselect All</div>
  </div>
</div>

<div id="fileBoxCandidates">
  <g:each in="${expFileCandidatesList?.sort { it.fileName }}" var="expFile">
    <p><g:checkBox name="cbFcsFileCandId_${expFile?.id}" checked="${expFile?.id in session.fCandSels}"/>&nbsp;
      <span>${expFile?.fileName}%{--&nbsp;${expFile.id}--}%</span>
    </p>
  </g:each>
</div>
<br/>
<br/>
<sec:ifAnyGranted roles="ROLE_Tester">
  <div style="padding-left: 20px">
    <g:each in="${experiment?.expFiles.sort { it.fileName }}" var="expFile">
      <g:each in="${expFile?.metaDatas}" var="mVals">
        <p>${expFile?.fileName} - ${mVals?.mdVal}</p>
      </g:each>
    </g:each>
  </div>
</sec:ifAnyGranted>


