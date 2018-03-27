<%@ page import="flowgate.Experiment; flowgate.Dataset" %>
<g:if test="${module}">
    <g:each in="${module.moduleParams.sort{it.id}.findAll{it.pBasic == true}}" var="moduleParam" >
        <g:if test="${moduleParam?.pType == 'ds'}">
            <div class="fieldcontain">
                <label for="mp-${moduleParam?.id}-ds">${moduleParam?.pKey} [dataset]</label>
                <g:if test="${moduleParam?.pType == 'ds'}">
                    <g:select style="width: 40%;display:inline" id="mp-${moduleParam?.id}-ds" from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds" optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" />
                </g:if>
            </div>
        </g:if>
        <g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
            <div class="fieldcontain">
                <label for="mp-${moduleParam?.id}">${moduleParam?.pKey} [dir/file]</label>
                %{--<g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'dir'}">--}%
                <g:if test="${moduleParam?.pType == 'dir'}">
                    <input type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" webkitdirectory directory multiple />
                </g:if>
                <g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'file'}">
                    <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </g:if>
                <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds'}">
                    <input type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </g:if>
            </div>
        </g:if>
    </g:each>
    <div class="tab-pane fade in active" id="basic">
        %{--BASIC OPTIONS--}%
        %{-- Always displayed ! --}%
    </div>
    <div class="tab-pane fade" id="advanced">
        %{--ADVANCED OPTIONS--}%
        <g:each in="${module.moduleParams.sort{it.id}.findAll{it.pBasic == false}}" var="moduleParam" >
            <g:if test="${moduleParam?.pType == 'ds'}">
                <div class="fieldcontain">
                    <label for="mp-${moduleParam?.id}-ds">${moduleParam?.pKey} [dataset]</label>
                    <g:if test="${moduleParam?.pType == 'ds'}">
                        <g:select style="width: 40%;display:inline" id="mp-${moduleParam?.id}-ds" from="${Dataset.findAllByExperiment(Experiment.get(params.eId))}" name="mp-${moduleParam?.id}-ds" optionKey="id" optionValue="name" value="${moduleParam?.defaultVal}" />
                    </g:if>
                </div>
            </g:if>
            <g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'dir' || moduleParam?.pType == 'file'}">
                <div class="fieldcontain">
                    <label for="mp-${moduleParam?.id}">${moduleParam?.pKey}</label>
                    %{--<g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'dir'}">--}%
                    <g:if test="${moduleParam?.pType == 'dir'}">
                        <input webkitdirectory directory multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                    </g:if>
                    <g:if test="${moduleParam?.pType == 'ds' || moduleParam?.pType == 'file'}">
                        <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                    </g:if>
                    <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file' && moduleParam?.pType != 'ds'}">
                        <input type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                    </g:if>
                </div>
            </g:if>
        </g:each>
    </div>
</g:if>