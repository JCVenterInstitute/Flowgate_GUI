<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'module.label', default: 'Module')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
%{--<a href="#edit-module" class="skip" tabindex="-1"><g:message code="default.link.skip.label"
                                                             default="Skip to content&hellip;"/></a>

<div class="nav" role="navigation">
  <ul>
    <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
    <li><g:link class="list" action="index"><g:message code="default.list.label" args="[entityName]"/></g:link></li>
    <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]"/></g:link></li>
  </ul>
</div>--}%

<div id="edit-module" class="container" role="main">
  <h1 class="page-header"><g:message code="default.edit.label" args="[entityName]"/></h1>
  <g:if test="${flash.message}">
    <div class="row justify-content-center ">
      <div class="alert alert-info text-center" role="alert">${flash.message}</div>
    </div>
  </g:if>
  <g:hasErrors bean="${this.module}">
    <ul class="errors" role="alert">
    <g:eachError bean="${this.module}" var="error">
      <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
    </g:eachError>
    </ul>
  </g:hasErrors>
  <g:form resource="${this.module}" method="PUT">
    <g:hiddenField name="version" value="${this.module?.version}"/>
    <div class="col-sm-4">
        <f:with bean="module">
            <f:field property="server" label="Server" required="true"/>
            <f:field property="label" label="Label" required="false"/>
            <f:field property="title" label="Title" required="true"/>
            <f:field property="name" label="Module or URN" required="true"/>
            <f:field property="descript" label="Module Description" required="false"/>
        </f:with>
      <input class="save btn btn-primary" type="submit" value="${message(code: 'default.button.update.label', default: 'Update')}"/>
    </div>
    <div class="col-sm-8">
    <fieldset class="form">
      <g:render template="templates/moduleParamsTbl" model="[module: this?.module]"/>
    </fieldset>
    </div>
  </g:form>
</div>
<script type="text/javascript">
  document.getElementById("Oserver").className = "form-control";
</script>
</body>
</html>
