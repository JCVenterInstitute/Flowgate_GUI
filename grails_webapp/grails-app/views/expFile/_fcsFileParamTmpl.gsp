<h4>Gate Drawing Settings for ${name}</h4>
<input id="selected-fcs-file" value="${id}" type="hidden"/>
<table class="responsive-table highlight centered">
  <thead>
  <tr>
    <th style="width: 10%;">Visible</th>
    <th style="width: 10%">Channel Id</th>
    <th style="width: 20%">Parameter Name</th>
    <th style="width: 20%">Marker Name</th>
    <th style="width: 40%">Transform</th>
  </tr>
  </thead>

  <tbody>
  <g:each status="i" in="${parameters}" var="parameter">
    <g:set var="parameterName" value="${parameter.replace(" ", "_")}"/>
    <tr>
      <td>
        <label>
          <input type="checkbox" class="filled-in" checked="checked" name="${parameterName}_visible"/><span></span>
        </label>
      </td>
      <td>${channelIds[i]}</td>
      <td>${parameter}</td>
      <td><input placeholder="${labels[i]}" id="${parameterName}_label" type="text" value="${labels[i]}" class="center-align" style="max-width: 100px"></td>
      <td>
        <label>
          <input name="${parameterName}_transform" type="radio" value="none" <g:if test="${transforms[i].transformType == null || transforms[i].transformType.equals("none")}">checked</g:if>/>
          <span>None</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="linear" <g:if test="${transforms[i].isPredefined || transforms[i].transformType.equals("linear")}">checked</g:if>/>
          <span>Linear</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="log" <g:if test="${transforms[i].isPredefined || transforms[i].transformType.equals("log")}">checked</g:if>/>
          <span>Log</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="logicle" <g:if test="${transforms[i].isPredefined || transforms[i].transformType.equals("logicle")}">checked</g:if>/>
          <span>Logicle</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="predefined" <g:if test="${transforms[i].transformType != null && transforms[i].isPredefined}">checked</g:if>/>
          <span>Predefined</span>
        </label>

        %{--None--}%
        <p <g:if test="${!transforms[i].transformType.equals("none") || transforms[i].transformType == null}">style="display: none;"</g:if> class="${parameterName}_transform_all ${parameterName}_transform_none">No transformation selected.</p>

        %{--Linear--}%
        <div <g:if test="${transforms[i].isPredefined || !transforms[i].transformType.equals("linear")}">style="display: none;"</g:if> class="${parameterName}_transform_all ${parameterName}_transform_linear">
          <div class="input-field col s3 offset-s3">
            <input id="${parameterName}_transform_linear_a" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("linear")}">value="${transforms[i].parameterValues.a}"</g:if>>
            <label for="${parameterName}_transform_linear_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_linear_t" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("linear")}">value="${transforms[i].parameterValues.t}"</g:if>>
            <label for="${parameterName}_transform_linear_t">T</label>
          </div>
        </div>

        %{--Log--}%
        <div <g:if test="${transforms[i].isPredefined || !transforms[i].transformType.equals("log")}">style="display: none;"</g:if> class="${parameterName}_transform_all ${parameterName}_transform_log">
          <div class="input-field col s3 offset-s3">
            <input id="${parameterName}_transform_log_m" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("log")}">value="${transforms[i].parameterValues.m}"</g:if>>
            <label for="${parameterName}_transform_log_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_log_t" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("log")}">value="${transforms[i].parameterValues.t}"</g:if>>
            <label for="${parameterName}_transform_log_t">T</label>
          </div>
        </div>

        %{--Logicle--}%
        <div <g:if test="${transforms[i].isPredefined || !transforms[i].transformType.equals("logicle")}">style="display: none;"</g:if> class="${parameterName}_transform_all ${parameterName}_transform_logicle">
          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_a" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("logicle")}">value="${transforms[i].parameterValues.a}"</g:if>>
            <label for="${parameterName}_transform_logicle_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_t" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("logicle")}">value="${transforms[i].parameterValues.t}"</g:if>>
            <label for="${parameterName}_transform_logicle_t">T</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_m" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("logicle")}">value="${transforms[i].parameterValues.m}"</g:if>>
            <label for="${parameterName}_transform_logicle_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_w" class="center-align validate" type="number" <g:if test="${transforms[i].transformType.equals("logicle")}">value="${transforms[i].parameterValues.w}"</g:if>>
            <label for="${parameterName}_transform_logicle_w">W</label>
          </div>
        </div>

        %{--Pre Defined--}%
        <div <g:if test="${transforms[i].transformType == null || !transforms[i].isPredefined}">style="display: none;"</g:if> class="${parameterName}_transform_all ${parameterName}_transform_predefined">
          <div class="input-field">
            <select name="${parameterName}_transform_predefined" id="${parameterName}_transform_predefined" class="fcs_param_transform">
              <g:each in="${predefinedList}" var="predefined">
                <option value="${predefined.transformName}" <g:if test="${transforms[i].transformName.equals(predefined.transformName)}">selected</g:if>>${predefined.transformName}</option>
              </g:each>
            </select>
            <label>Select a predefined transformation</label>
          </div>
        </div>
      </td>
    </tr>
  </g:each>
  </tbody>
</table>

<script>
  $('input[type=radio][name$="_transform"]').change(function () {
    $("." + this.name + "_all").hide();
    $("." + this.name + "_" + this.value).show();
  });
</script>
