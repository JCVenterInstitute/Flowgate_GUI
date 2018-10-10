%{--<div class="form-group">--}%
  %{--<label for="${property}">${label} ${required ? '*' : ''}</label>--}%
  %{--<input type="text" class="form-control" id="${property}" name="${property}" placeholder="${label}" ${required}>--}%
%{--</div>--}%
<%@ page defaultCodec="html" %>
<div class="form-group">
  <div class="fieldcontain ${required ? 'required' : ''}">
    <label for="${property}" style="width: 100%;text-align: left">${label}  ${required ? '*' : ''}</label>
    <input type="text" class="form-control" name="${property}" ${required ? 'required=\"\"' : ''} value="${value}" />
  </div>
</div>