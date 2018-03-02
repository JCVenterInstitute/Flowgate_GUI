<g:if test="${module}">
    %{--${module.moduleParams.dump()}--}%
    <g:each in="${module.moduleParams.sort{it.id}.findAll{it.pBasic == true}}" var="moduleParam" >
        <div class="fieldcontain">
            <label for="mp-${moduleParam?.id}">${moduleParam?.pKey}</label>
            <g:if test="${moduleParam?.pType == 'dir'}">
                %{--<input type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" accept="text/*" webkitdirectory directory multiple />--}%
                <input type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}" webkitdirectory directory multiple />
            </g:if>
            <g:if test="${moduleParam?.pType == 'file'}">
                %{--<input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" accept="text/*" value="${moduleParam?.defaultVal}"/>--}%
                <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
            <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file'}">
                <input type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </g:if>
        </div>
        %{--<p>${moduleParam?.defaultVal}</p>--}%
        %{--<div class="fieldcontain">
            <label for="mp-${moduleParam?.id}">${moduleParam?.pKey}</label>
            <g:textField style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
        </div>--}%
    </g:each>
    <div class="tab-pane fade in active" id="basic">
        %{--BASIC OPTIONS--}%
        %{--
        <g:each in="${module.moduleParams.sort{it.id}.findAll{it.pBasic == true}}" var="moduleParam" >
            <div class="fieldcontain">
                <label for="mp-${moduleParam?.id}">${moduleParam?.pKey}</label>
                <g:textField style="width: 60%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
            </div>
        </g:each>
        --}%
    </div>
    <div class="tab-pane fade" id="advanced">
        %{--ADVANCED OPTIONS--}%
        <g:each in="${module.moduleParams.sort{it.id}.findAll{it.pBasic == false}}" var="moduleParam" >
            <div class="fieldcontain">
                <label for="mp-${moduleParam?.id}">${moduleParam?.pKey}</label>
                %{--<g:field type="${moduleParam?.pType == 'file' ? 'file' : 'text'}" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-{moduleParam?.id}" value="${moduleParam?.defaultVal}"/>--}%
                <g:if test="${moduleParam?.pType == 'dir'}">
                    <input webkitdirectory directory multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </g:if>
                <g:if test="${moduleParam?.pType == 'file'}">
                    <input multiple type="file" style="width: 40%;display:inline" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </g:if>
                <g:if test="${moduleParam?.pType != 'dir' && moduleParam?.pType != 'file'}">
                    <input type="text" style="width: 40%;" id="mp-${moduleParam?.id}" name="mp-${moduleParam?.id}" value="${moduleParam?.defaultVal}"/>
                </g:if>
            </div>
        </g:each>
    </div>
</g:if>