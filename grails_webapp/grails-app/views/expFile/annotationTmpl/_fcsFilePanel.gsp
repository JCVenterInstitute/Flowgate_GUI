<%@ page import="flowgate.ExperimentMetadataValue" %>
<style>
#fileBox {
    /*margin-right: auto;*/
    /*margin-top: 20px;*/
    /*background-color: #eefaff;*/
    /*box-shadow: #ee50e3 1px 0;*/
    border: black 1px solid;
    padding: 10px 10px 10px 10px;
    overflow-y: scroll;
    overflow-x: hidden;
    max-height: 700px;
}
</style>
<p>&nbsp;</p>
<p>&nbsp;</p>
%{--<p>sels=${session.sels}</p>--}%
<div id="fileBox">
    <g:each in="${experiment?.expFiles.sort{it.fileName}}" var="expFile" >
        <p><g:checkBox name="fcsFileId_${expFile.id}_chb"  checked="${expFile.id in session.fSels}"/>&nbsp;${expFile.fileName}%{--&nbsp;${expFile.id}--}%</p>
    </g:each>
    <br/>
    %{--<div class="row">--}%
        %{--<button class="pull-left btn btn-default" href="${g.createLink(controller: 'expFile', action: 'selAllFcs', id: experiment?.id)}">--}%%{--<i class="fa fa"></i>--}%%{--Select All</button>--}%
        %{--<button class="pull-right btn btn-default" href="${g.createLink(controller: 'expFile', action: 'deselAllFcs', id: experiment?.id)}">--}%%{--<i class="fa fa"></i>--}%%{--Deselect All</button>--}%
        <g:actionSubmit class="pull-left btn btn-default" value="selAllFcs" id="${experiment?.id}" >%{--<i class="fa fa"></i>--}%Select All</g:actionSubmit>
        <g:actionSubmit class="pull-right btn btn-default" value="deselAllFcs" id="${experiment?.id}">%{--<i class="fa fa"></i>--}%Deselect All</g:actionSubmit>
    %{--</div>--}%
</div>
<br/>
<br/>
<br/>
<br/>
<br/>
<hr/>
<br/>
<br/>
<div style="padding-left: 20px">
<g:each in="${experiment.expFiles.sort{it.fileName}}" var="expFile" >
    <g:each in="${expFile.metaDatas}" var="mVals">
        %{--<p>${expFile.fileName} - ${ExperimentMetadataValue.find(mVals.mdVal)}</p>--}%
        <p>${expFile.fileName} - ${mVals.mdVal}</p>
    </g:each>
</g:each>
</div>


