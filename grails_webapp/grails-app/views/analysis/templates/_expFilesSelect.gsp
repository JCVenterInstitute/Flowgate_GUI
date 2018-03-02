<%@ page import="flowgate.ExpFile; flowgate.Experiment" %>
<g:each var="expFile" in="${ExpFile.findAllByExperimentAndIsActive(Experiment.get(eId.toLong()), true)}" >
    <p>
    <g:checkBox name="${expFile.fileName}" value="${expFileSelected.find {it.toLong() == expFile.id} != null}" />&nbsp;&nbsp;${expFile.title}
    %{--->--}%
    %{--&nbsp;&nbsp;${expFile.filePath}/${expFile.fileName}--}%
    %{--&nbsp;&nbsp;${expFile.dateCreated}--}%
    </p>
</g:each>