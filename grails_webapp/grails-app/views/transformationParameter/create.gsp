<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'transformation.label', default: 'Transformation Parameter')}"/>
  <title><g:message code="default.create.label" args="[entityName]"/></title>
</head>

<body>

<g:hasErrors bean="${this.transformationParameter}">
  <div class="row">
    <div class="alert alert-danger col-xs-12 col-sm-6" role="alert">
      <g:eachError bean="${this.transformationParameter}" var="error">
        <li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </div>
  </div>
</g:hasErrors>


<h2><g:message code="default.create.label" args="[entityName]"/></h2>

<div class="row">
  <g:form action="save" class="col s6">
    <f:with bean="transformationParameter">
      <div class="row">
        <f:field property="transformName" required="true"/>
        <div class="input-field">
          <select name="transformType" id="transformType">
            <option value="none">None</option>
            <option value="linear">Linear</option>
            <option value="log">Log</option>
            <option value="logicle">Logicle</option>
          </select>
          <label>Select a transformation type</label>
        </div>

        %{--Linear--}%
        <div style="display: none;" class="row transform_values transform_value_linear">
          <div class="input-field col s3">
            <input id="transform_linear_a" name="transform_linear_a" class="validate" type="number">
            <label for="transform_linear_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="transform_linear_t" name="transform_linear_t" class="validate" type="number">
            <label for="transform_linear_t">T</label>
          </div>
        </div>

        %{--Log--}%
        <div style="display: none;" class="row transform_values transform_value_log">
          <div class="input-field col s3">
            <input id="transform_log_m" name="transform_log_m" class="validate" type="number">
            <label for="transform_log_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="transform_log_t" name="transform_log_t" class="validate" type="number">
            <label for="transform_log_t">T</label>
          </div>
        </div>

        %{--Logicle--}%
        <div style="display: none;" class="row transform_values transform_value_logicle">
          <div class="input-field col s3">
            <input id="transform_logicle_a" name="transform_logicle_a" class="validate" type="number">
            <label for="transform_logicle_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="transform_logicle_t" name="transform_logicle_t" class="validate" type="number">
            <label for="transform_logicle_t">T</label>
          </div>

          <div class="input-field col s3">
            <input id="transform_logicle_m" name="transform_logicle_m" class="validate" type="number">
            <label for="transform_logicle_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="transform_logicle_w" name="transform_logicle_w" class="validate" type="number">
            <label for="transform_logicle_w">W</label>
          </div>
        </div>
      </div>
    </f:with>
    <div class="row">
      <div class="input-field">
        <button type="submit" class="btn waves-effect waves-light">Create a Transformation Parameter</button>
        <g:if test="${!flash.firstTransformation}">
          <a href="${createLink(controller: 'transformationParameter', action: 'index')}" class="btn-flat">Return to Transformation List</a>
        </g:if>
      </div>
    </div>
  </g:form>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
  });

  $('#transformType').change(function () {
    $(".transform_values").hide();
    $(".transform_value_" + this.value).show();
  });
</script>
</body>
</html>
