<h4>${name}</h4>
<input id="selected-fcs-file" value="${id}" type="hidden"/>
<table class="responsive-table highlight centered">
  <thead>
  <tr>
    <th style="width: 10%;">Visible</th>
    <th style="width: 20%">Parameter</th>
    <th style="width: 20%">Marker Name</th>
    <th style="width: 50%">Transform</th>
  </tr>
  </thead>

  <tbody>
  <g:each status="i" in="${parameters}" var="parameter">
    <g:set var="parameterName" value="${parameter.replace(" ", "_")}"/>
    <tr>
      <td>
        <label>
          <input type="checkbox" class="filled-in" checked="checked" name="${parameterName}_visible"/><span></span>
        </label></td>
      <td>${parameter}</td>
      <td><input placeholder="${labels[i]}" id="${parameterName}_label" type="text" value="${labels[i]}" class="center-align" style="max-width: 100px"></td>
      <td>
        <label>
          <input name="${parameterName}_transform" type="radio" value="none" checked/>
          <span>None</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="linear"/>
          <span>Linear</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="log"/>
          <span>Log</span>
        </label>
        <label>
          <input name="${parameterName}_transform" type="radio" value="logicle"/>
          <span>Logicle</span>
        </label>

        %{--None--}%
        <p class="${parameterName}_transform_all ${parameterName}_transform_none">No transformation selected.</p>

        %{--Linear--}%
        <div style="display: none;" class="${parameterName}_transform_all ${parameterName}_transform_linear">
          <div class="input-field col s3 offset-s3">
            <input id="${parameterName}_transform_linear_a" class="center-align validate" type="number">
            <label for="${parameterName}_transform_linear_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_linear_t" class="center-align validate" type="number">
            <label for="${parameterName}_transform_linear_t">T</label>
          </div>
        </div>

        %{--Log--}%
        <div style="display: none;" class="${parameterName}_transform_all ${parameterName}_transform_log">
          <div class="input-field col s3 offset-s3">
            <input id="${parameterName}_transform_log_m" class="center-align validate" type="number">
            <label for="${parameterName}_transform_log_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_log_t" class="center-align validate" type="number">
            <label for="${parameterName}_transform_log_t">T</label>
          </div>
        </div>

        %{--Logicle--}%
        <div style="display: none;" class="${parameterName}_transform_all ${parameterName}_transform_logicle">
          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_a" class="center-align validate" type="number">
            <label for="${parameterName}_transform_logicle_a">A</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_t" class="center-align validate" type="number">
            <label for="${parameterName}_transform_logicle_t">T</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_m" class="center-align validate" type="number">
            <label for="${parameterName}_transform_logicle_m">M</label>
          </div>

          <div class="input-field col s3">
            <input id="${parameterName}_transform_logicle_w" class="center-align validate" type="number">
            <label for="${parameterName}_transform_logicle_w">W</label>
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
