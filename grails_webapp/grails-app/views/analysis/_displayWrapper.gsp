%{--<<<<<<< HEAD   TODO remove after testing and uncomment last line--}%
<div class="form-control-plaintext" >${value}
  <g:if test="${property == 'analysisName'}">
    &nbsp;&nbsp;
    <sec:ifAnyGranted roles="ROLE_Admin,ROLE_Administrator,ROLE_Tester,ROLE_Acs">
      <a href="${g.createLink(controller: 'analysis', action: 'del', params: [id: bean?.id, jobNr: bean?.jobNumber])}">
        <i class="fa fa-trash"></i>
      </a>
    </sec:ifAnyGranted>
  </g:if>
</div>
%{--=======--}%
%{--<div class="form-control-plaintext" >${value}</div>--}%
