<div class="form-group">
  <label class="col-sm-2 control-label" for="${property}">${label} ${required ? '*' : ''}</label>
  <div class="col-sm-10">
    <input type="text" class="form-control" id="${property}" name="${property}" placeholder="${label}" ${required} value="${value}">
  </div>
</div>