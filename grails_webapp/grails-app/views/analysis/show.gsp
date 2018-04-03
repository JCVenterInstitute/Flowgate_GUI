<!DOCTYPE html>
<html>
<head>
    <meta name="layout" content="main"/>
    <g:set var="entityName" value="${message(code: 'analysis.label', default: 'Analysis')}"/>
    <title><g:message code="default.show.label" args="[entityName]"/></title>
</head>

<body>
<a href="#show-analysis" class="skip" tabindex="-1"><g:message code="default.link.skip.label"
                                                               default="Skip to content&hellip;"/></a>

<div class="nav" role="navigation">
    <ul>
        %{--<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>--}%
        %{--<li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]" /></g:link></li>--}%
        %{--<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>--}%
        <li><a class="home" href="${createLink(uri: '/experiment/index?eId=' + this.analysis.experiment.id)}"><g:message
             code="default.home.label"/></a></li>
        <li><g:link class="list" action="index" params="[eId: this.analysis.experiment.id]"><g:message
             code="default.list.label" args="[entityName]"/></g:link></li>
        <li><g:link class="create" action="create" params="[eId: this.analysis.experiment.id]"><g:message
             code="default.new.label" args="[entityName]"/></g:link></li>
    </ul>
</div>

<div id="show-analysis" class="content scaffold-show" role="main">
  <h1><g:message code="default.show.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="message" role="status">${flash.message}</div>
  </g:if>
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