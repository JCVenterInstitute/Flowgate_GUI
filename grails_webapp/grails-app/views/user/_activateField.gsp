<%@ page import="flowgate.Role" %>
%{--<div ${(user?.authorities.find {it == flowgate.Role.findByAuthority('ROLE_NewUser')}) ? 'onclick=activate('+user?.id+')' :''} >--}%
<i style="cursor: pointer; color: ${(user?.authorities.find {
  it == flowgate.Role.findByAuthority('ROLE_NewUser')
}) ? 'red' : 'green'}"
   class="fa fa-${(user?.authorities.find { it == flowgate.Role.findByAuthority('ROLE_NewUser') }) ? 'times' : 'check'}">
</i>
%{--</div>--}%