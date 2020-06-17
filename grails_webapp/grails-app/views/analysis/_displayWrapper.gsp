%{--<input type="text" readonly class="form-control-plaintext" id="${property}" value="${value}">--}%
%{--<<<<<<< HEAD   TODO remove after testing--}%
<div class="form-control-plaintext" >${value}
  <g:if test="${property == 'analysisName'}">
    &nbsp;&nbsp;
    <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_Tester,ROLE_Acs">
    %{--              &nbsp;&nbsp;&nbsp;&nbsp;--}%
      <a href="${g.createLink(controller: 'analysis', action: 'del', params: [id: bean?.id, jobNr: bean?.jobNumber])}">
        <i class="fa fa-trash"></i>
        %{--                  &nbsp;<g:message code="analysis.delete.label" default="delete" />--}%
      </a>
    </sec:ifAnyGranted>
  </g:if>
</div>
%{--=======--}%
<div class="form-control-plaintext" >${value}</div>
