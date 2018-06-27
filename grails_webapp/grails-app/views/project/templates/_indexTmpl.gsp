<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${project}">
  <f:with bean="project">
    <h1 class="page-header"><f:display property="title"/></h1>

    <g:if test="${poject?.id == session.projectEditModeId?.toLong()}">
      <textarea id="projectDescription" class="form-control" rows="8" cols="130" onchange="projectDescriptionInputBlur(${project?.id}, this.value)"
                onfocus="this.select()">${project?.description}</textarea>
    </g:if>
    <g:else>
      <textarea class="form-control" readonly="true" rows="8" cols="130">${project?.description}</textarea>
    </g:else>
  </f:with>
</g:if>

<div class="pull-right pt-3">
  <a href="/flowgate/experiment/create?pId=${project?.id}" class="btn btn-info">
    <i class="glyphicon glyphicon-plus"></i>Add Experiment
  </a>
  %{--<a href="/flowgate/experiment/createFromTemplate?pId=${project?.id}" class="btn btn-info">
    <i class="glyphicon glyphicon-plus"></i>Add From Template
  </a>--}%
</div>

<h1 class="sub-header">Experiments</h1>

<div class="row">
  <div id="expContent">
    <g:if test="${!session?.expCardView ?: true}">
      <g:render template="templates/expCardsTmpl"/>
    </g:if>
    <g:else>
      <g:render template="templates/expListTmpl"/>
    </g:else>
  </div>
</div>