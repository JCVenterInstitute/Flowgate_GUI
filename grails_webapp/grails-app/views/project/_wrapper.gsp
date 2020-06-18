<div class="input-field col s12">
  <g:if test="${type == 'textarea'}">
    <textarea name="${property}" class="materialize-textarea" required></textarea>
  </g:if>
  <g:else>
    <input type="text" name="${property}" required/>
  </g:else>
  <label for="${property}">${label}</label>
</div>