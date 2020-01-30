<!DOCTYPE html>
<html>
<head>
    <meta name="layout" content="main"/>
    <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
    <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>

<div id="show-analysis" class="content scaffold-show" role="main">
  <ul class="breadcrumb">
    <li><a href="${createLink(controller: 'project', action: 'index', params: [pId: analysis?.experiment?.project?.id])}" title="${analysis?.experiment?.project?.title}">${analysis?.experiment?.project?.title}</a></li>
    <li><a href="${createLink(controller: 'experiment', action: 'index', params: [eId: analysis?.experiment?.id])}" title="${analysis?.experiment?.title}">${analysis?.experiment?.title}</a></li>
    <li><a href="${createLink(controller: 'analysis', action: 'index', params: [eId: analysis?.experiment?.id])}">List of Analysis Task</a></li>
    <li class="active">Analysis</li>
  </ul>
  <h1><g:message code="default.show.label" args="[entityName]"/></h1>
  <f:display bean="analysis"/>
  <g:form resource="${this.analysis}" method="DELETE">
    <fieldset class="buttons">
      <g:link class="edit" action="edit" resource="${this.analysis}">
        <g:message code="default.button.edit.label" default="Edit"/>
      </g:link>
      <input class="delete" type="submit"
             value="${message(code: 'default.button.delete.label', default: 'Delete')}"
             onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');"/>
    </fieldset>
  </g:form>
</div>
</body>
</html>