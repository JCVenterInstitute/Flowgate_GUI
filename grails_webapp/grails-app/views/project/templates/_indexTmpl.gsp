<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<div class="container" >
    <g:if test="${project}">
        %{--<div id="show-project" class="content scaffold-show" role="main">--}%
        %{--<h1><g:message code="default.show.label" args="[entityName]" /></h1>--}%
            %{--<g:if test="${flash.message}">
                <div class="message" role="status">${flash.message}</div>
            </g:if>--}%
        %{--<f:display bean="project" />--}%
            <f:with bean="project">
                <h1 class="text-center"><strong><f:display property="title" /></strong></h1>
                <br/>
                %{--<h2 class="text-center"><f:display property="description" /></h2>--}%
                <div class="jumbotron">
                    <g:if test="${poject?.id == session.projectEditModeId?.toLong()}">
                        <textarea id="projectDescription" class="form-control" rows="10" cols="130" onchange="projectDescriptionInputBlur(${project?.id}, this.value)" onfocus="this.select()" >${project?.description}</textarea>
                    </g:if>
                    <g:else>
                        <textarea class="form-control" readonly="true" rows="10" cols="130" >${project?.description}</textarea>
                    </g:else>
                </div>
            %{--<f:display property="title" />--}%
            %{--<f:display property="title" />--}%
            %{--<f:display property="title" />--}%
            </f:with>

        %{--<g:form resource="${this.project}" method="DELETE">
            <fieldset class="buttons">
                <g:link class="edit" action="edit" resource="${this.project}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
                <input class="delete" type="submit" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
            </fieldset>
        </g:form>--}%
        %{--</div>--}%
    </g:if>


    %{--<div class="row" style="padding: 0;max-width: none;">--}%
    <div class="row">
        <div class="pull-right">
            %{--
            <div class="btn btn-primary">
                <g:link class="edit" action="edit" id="${project.id}">
                    <i class="glyphicon glyphicon-edit"></i>Edit Project
                </g:link>
            </div>
            --}%
            <div class="btn btn-primary">
                %{--
                <g:link class="create" controller="experiment" action="create" params="[pId: project?.id]">
                    <i class="glyphicon glyphicon-plus"></i>Add Experiment
                </g:link>
                --}%
                <a class="create" href="/flowgate/experiment/create?pId=${project?.id}">
                    <i class="glyphicon glyphicon-plus"></i>Add Experiment
                </a>
            </div>
            <div class="btn btn-primary">
                %{--<a href="${createLink(controller: 'experiment', action: 'createFromTemplate', params:[pId: project?.id] )}" >--}%
                <a href="/flowgate/experiment/createFromTemplate?pId=${project?.id}" >
                    <i class="glyphicon glyphicon-plus"></i>Add From Template
                </a>
            </div>
        </div>
    </div>
    <div class="row" style="padding: 0;max-width: none;">
    %{--<div class="row">--}%
        <div id="expContent">
            <g:if test="${!session?.expCardView ?: true}">
                <g:render template="templates/expCardsTmpl" />
            </g:if>
            <g:else>
                <g:render template="templates/expListTmpl" />
            </g:else>
        </div>
    </div>
</div>