<div class="input-field col s12">
  <g:if test="${type == 'textarea'}">
    <textarea name="${property}" id="${property}" class="materialize-textarea" <g:if test="${required}">required</g:if>>${value}</textarea>
  </g:if>
  <g:else>
    <input type="${email ? 'email' : password ? 'password' : 'text'}" name="${property}" id="${property}" ${property} <g:if test="${required}">required</g:if> value="${value}"/>
  </g:else>
  <label for="${property}">${label}<g:if test="${required}">*</g:if></label>
</div>
