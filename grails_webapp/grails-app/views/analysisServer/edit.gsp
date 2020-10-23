<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}"/>
  <title><g:message code="default.edit.label" args="[entityName]"/></title>
</head>

<body>
<h2><g:message code="default.edit.label" args="[entityName]"/></h2>

<div class="row">
  <g:form resource="${analysisServer}" method="PUT" class="col s6">
    <div class="row">
      <g:hiddenField name="version" value="${this.analysisServer?.version}"/>
      <g:hasErrors bean="${user}">
        <div class="alert alert-danger">
          <g:eachError var="err" bean="${user}">
            <li><g:message error="${err}"/></li>
          </g:eachError>
        </div>
      </g:hasErrors>

      <f:with bean="analysisServer">
        <div class="col s8">
          <div class="row">
            <f:field property="name" label="Server Name" required="true" value="${analysisServer.name}"/>
            <f:field property="url" required="true" value="${analysisServer.url}"/>

            <div class="input-field col s8">
              <select name="platform" id="platform">
                <option value="1">GenePattern</option>
                <option value="2">ImmportGalaxy</option>
              </select>
              <label>Select a Platform</label>
            </div>

            <f:field property="userName" label="Username" required="true" value="${analysisServer.userName}"/>
          </div>
        </div>
      </f:with>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Update Server</button>
        <a class="btn-flat modal-trigger" href="#delete-analysisserver-modal">Delete</a>
        <g:link controller="analysisServer" action="index" class="btn-flat">Return to Analysis Server List</g:link>
      </div>
    </div>
  </g:form>

  <g:form resource="${analysisServer}" method="PUT" action="updatePassword" class="col s6" onsubmit="return validatePass()">
    <div class="row">
      <g:if test="${flash.passError}">
        <div class="alert alert-danger">${flash.passError}</div>
      </g:if>

      <div class="input-field col s8">
        <input type="password" name="oldpass" id="oldpass" required>
        <label for="oldpass">Old Password*</label>
      </div>

      <div class="input-field col s8">
        <input type="password" name="newpass" id="newpass" required>
        <label for="newpass">New Password*</label>
      </div>

      <div class="input-field col s8">
        <button type="submit" class="btn waves-effect waves-light">Update Password</button>
      </div>
    </div>
  </g:form>
</div>

<div id="delete-analysisserver-modal" class="modal">
  <div class="modal-content">
    <h4>Confirm to delete analysis server</h4>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <g:link action="delete" resource="${analysisServer}" class="modal-close waves-effect waves-light btn-flat">Confirm</g:link>
  </div>
</div>


<script>
  document.addEventListener('DOMContentLoaded', function () {
    var modalElems = document.querySelectorAll('.modal');
    M.Modal.init(modalElems);

    var elems = document.querySelector('#platform');
    elems.value = "${analysisServer.platform}";
    var instances = M.FormSelect.init(elems);
  });
</script>
</body>
</html>
