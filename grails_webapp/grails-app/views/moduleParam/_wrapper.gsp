%{--templatesWrapper--}%
<div class="form-group">
  <label for="${property}" class="col-sm-4 control-label">${label} ${required ? '*' : ''}</label>

  <div class="col-sm-8">
    <input type="text" class="form-control" id="${property}" name="${property}" value="${value}" placeholder="${label}" ${required}>
  </div>
</div>