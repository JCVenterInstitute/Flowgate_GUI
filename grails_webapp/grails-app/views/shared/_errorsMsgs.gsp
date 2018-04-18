<g:hasErrors bean="${bean}">
  <div class="row justify-content-center">
    <div class="alert alert-danger text-center" role="alert">
      <g:eachError bean="${bean}" var="error">
        <li<g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
      </g:eachError>
    </div>
  </div>
</g:hasErrors>