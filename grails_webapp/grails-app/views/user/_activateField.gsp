<%@ page import="flowgate.Role" %>
<i style="color: ${(user?.authorities.find {it == flowgate.Role.findByAuthority('ROLE_NewUser')}) ? 'red' : 'green'}" class="material-icons">
  ${(user?.authorities.find { it == flowgate.Role.findByAuthority('ROLE_NewUser') }) ? 'clear' : 'check'}
</i>