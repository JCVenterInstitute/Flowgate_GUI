<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="wTreeSideBar"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>
<g:render template="/shared/nav"/>
<content tag="topBtnBar">
  <div id="topBtnBar" class="row">
    %{--
    <a href="#create-experiment" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
    <div class="nav" role="navigation">
        <ul>
            <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
            <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
        </ul>
    </div>
    --}%
    %{--
    <g:if test="${experiment?.id == session.experimentEditModeId?.toLong()}">
        <div class="pull-right btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">done</div>
    </g:if>
    <g:else>
        <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
            <div class="pull-right btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">
                <i class="glyphicon glyphicon-pencil"></i>&nbsp;edit
            </div>
        </sec:ifAnyGranted>
        <sec:ifNotGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentEdit">
            <g:isOwner object="experiment" objectId="${experiment?.id}">
                <div class="pull-right btn btn-info" onclick="toggleExpEditMode(${experiment?.id})">
                    <i class="glyphicon glyphicon-pencil"></i>&nbsp;edit
                </div>
            </g:isOwner>
        </sec:ifNotGranted>

        <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
            <g:link class="pull-right btn btn-info" controller="experiment" action="delete" id="${experiment?.id}" >
                <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
            </g:link>
        </sec:ifAnyGranted>
        <sec:ifNotGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExperimentDelete">
            <g:isOwner object="experiment" objectId="${experiment?.id}">
                <g:link class="pull-right btn btn-info" controller="experiment" action="delete" resource="${experiment}" >
                    <i class="glyphicon glyphicon-trash"></i>&nbsp;Delete
                </g:link>
            </g:isOwner>
        </sec:ifNotGranted>
    </g:else>
    --}%
    %{--
    <div class="pull-right">  </div>
    --}%
  </div>
  <g:if test="${flash.message}">
    <div class="row justify-content-center ">
      <div class="alert alert-info text-center" role="alert">${flash.message}</div>
    </div>
  </g:if>
  <g:hasErrors bean="${this.experiment}">
    <div class="row">
      <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
      <g:eachError bean="${this.experiment}" var="error">
        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
      </div>
    </div>
  </g:hasErrors>
</content>
<content tag="treeView">
  <div id="projTree">
    <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]"/>
  </div>
</content>
<content tag="pgContent">
  <div id="pageContent">
    <g:render template="templates/createTmpl" model="[project: project, projectList: projectList, experiment: experiment, experimentList: experimentList]"/>
  </div>
</content>

</body>
</html>
