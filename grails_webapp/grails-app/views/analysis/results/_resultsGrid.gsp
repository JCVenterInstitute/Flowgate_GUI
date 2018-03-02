<strong>Results</strong>
<br/>
<div class="btn btn-default" onclick="alert('not implemented yet');">Download Results as Zip-file</div>
<br/>
<ul>
    <g:set var="colNo" value="${4}" />
    <g:each in="${jobResult.outputFiles.findAll{(it.kind.contains('png') || it.kind.contains('jpg') || it.kind.contains('jpeg'))}}" var="outputFile" status="i" >
            <g:if test="${i+1 % colNo == 0}">
            <div class="row" style="max-width: 100%">
            </g:if>
            <div class="col-sm-${12.intdiv(colNo)}" >
                <p class="">${outputFile?.path}</p>
                <p style="padding-left: 10px"><img src="${outputFile?.link?.href}" alt="${outputFile?.filename}" width="70%"/></p>
                <p style="padding-left: 35%"><a href="${g.createLink(controller: 'analysis', action: 'downloadFile', params: [analysisId: this?.analysis?.id, filename: outputFile?.path, fileLink: outputFile?.link?.href, outputFile: outputFile ])}" >%{--${outputFile?.path}&nbsp;--}%<i class="fa fa-floppy-o fa-lg"></i></a></p></li>
                %{--<p><a href="${outputFile?.link?.href}" target="_blank">View On Server&nbsp;<i class="glyphicon glyphicon-eye-open"></i></a></p>--}%
            </div>
            <g:if test="${i+1 % colNo == 0}">
            </div>
            <br/>
            <hr/>
            <br/>
            </g:if>
    </g:each>
</ul>
<br/>
<br/>