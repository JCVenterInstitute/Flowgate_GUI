<%@ page defaultCodec="html" %>
<div class="input-field">
  <label for="${property}">${label}  ${required ? '*' : ''}</label>
  <input type="text" name="${property}" ${required ? 'required=\"\"' : ''} value="${value}"/>
</div>