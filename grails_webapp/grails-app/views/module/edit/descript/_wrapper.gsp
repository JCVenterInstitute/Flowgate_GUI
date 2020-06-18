<%@ page defaultCodec="html" %>
<div class="input-field col s12">
  <label for="${property}">${label}  ${required ? '*' : ''}</label>
  <g:textArea name="${property}" required="${required ? 'true' : ''}" value="${value}" class="materialize-textarea" style="height: 300px"/>
</div>