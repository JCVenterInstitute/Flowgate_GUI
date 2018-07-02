<f:table collection="${module.moduleParams}" properties="['pKey','defaultVal','pBasic','pType']" />
%{--TODO test this!! --}%
%{--<a href="/flowgate/moduleParam/create?module.id=2" class="btn btn-success">Add ModuleParam</a>--}%
<a href="${createLink(controller: 'moduleParam', action: 'create', params: ['module.id': module.id])}" class="btn btn-success">Add ModuleParam</a>