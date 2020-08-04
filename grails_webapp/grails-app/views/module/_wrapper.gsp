<%@ page defaultCodec="html" %>
<div class="input-field col s12">
  <input type="text" name="${property}" <g:if test="${required}">required</g:if> value="${value}"/>
  <label for="${property}">${label}<g:if test="${required}">*</g:if></label>
</div>
