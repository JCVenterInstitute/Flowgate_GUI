%{--<input type="text" readonly class="form-control-plaintext" id="${property}" value="${value}">--}%
<<<<<<< HEAD
<div class="form-control-plaintext" >${value}
%{--  TODO remove after testing !!!--}%
  <g:if test="${property == 'analysisName'}">
    &nbsp;&nbsp;
    <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_Tester,ROLE_Acs">
      <a href="${g.createLink(controller: 'analysis', action: 'del', params: [id: bean?.id, jobNr: bean?.jobNumber])}">
        <i class="fa fa-trash"></i>
      </a>
    </sec:ifAnyGranted>
  </g:if>
</div>
<div class="form-control-plaintext" >${value}</div>
