<%@ page defaultCodec="html" %>
<div class="form-group">
  <div class="fieldcontain ${required ? 'required' : ''}">
    <label for="${property}" style="width: 100%;text-align: left">${label}  ${required ? '*' : ''}</label>
    <g:textArea name="${property}" required="${required ? 'true' : ''}" value="${value}" cols="42" rows="5"/>
  </div>
</div>