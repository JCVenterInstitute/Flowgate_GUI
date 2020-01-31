<%@ page defaultCodec="html" %>
<div class="input-field col s12">
  <label for="${property}">${label}  ${required ? '*' : ''}</label>
  <input type="text" name="${property}" ${required ? 'required=\"\"' : ''} value="${value}"/>
</div>