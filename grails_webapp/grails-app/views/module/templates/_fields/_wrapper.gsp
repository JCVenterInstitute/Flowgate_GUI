%{--fieldsWrapper--}%
<div class="form-group">
  <label for="${property}">${label} ${required ? '*' : ''}</label>
  <input type="text" class="form-control" id="${property}" name="${property}" placeholder="${label}" ${required}>
</div>