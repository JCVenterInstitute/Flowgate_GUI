<%@ page import="flowgate.Dataset; flowgate.ExperimentMetadataValue" %>
<style>
  #fileBox {
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
</style>
<p>&nbsp;</p>
<div class="row">
  <div class="col-sm-12">
    <g:actionSubmit class="pull-left btn btn-default" value="selAllFcs" id="${experiment?.id}" >%{--<i class="fa fa"></i>--}%Select All</g:actionSubmit>
    <g:actionSubmit class="pull-right btn btn-default" value="deselAllFcs" id="${experiment?.id}">%{--<i class="fa fa"></i>--}%Deselect All</g:actionSubmit>
  </div>
</div>
%{--<p>&nbsp;</p>--}%
%{--<p>sels=${session.sels}</p>--}%
<div id="fileBox">
    <g:each in="${expFileList?.sort{it.fileName}}" var="expFile" >
        <p><g:checkBox name="fcsFileId_${expFile.id}_chb"  checked="${expFile.id in session.fSels}"/>&nbsp;
          <span id="" style="font-weight:${(expFile.id in ds?.expFiles*.id) ? 'bold' : 'normal'}" >${expFile.fileName}%{--&nbsp;${expFile.id}--}%</span>
          <g:if test="${expFile.id in ds?.expFiles*.id}">
            <i class="glyphicon glyphicon-ok" ></i>
          </g:if>
        </p>
    </g:each>
</div>
<br/>
<br/>
<sec:ifAnyGranted roles="ROLE_Tester">
  <div style="padding-left: 20px">
  <g:each in="${experiment.expFiles.sort{it.fileName}}" var="expFile" >
    <g:each in="${expFile.metaDatas}" var="mVals">
      <p>${expFile.fileName} - ${mVals.mdVal}</p>
    </g:each>
  </g:each>
  </div>
</sec:ifAnyGranted>


