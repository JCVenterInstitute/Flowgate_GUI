%{--templatesWrapper--}%
<div class="input-field col s12">
  <input type="text" name="${property}" id="${property}" value="${value}" ${required}>
  <label for="${property}">${label} ${required ? '*' : ''}</label>
</div>
