%{--<g:set var="sss" bean="springSecurityService"/>--}%
<%@ page import="flowgate.AnalysisServer" defaultCodec="html" %>
<div class="input-field col s12">
  %{--<input type="text" class="form-control" name="${property}" ${required ? 'required=\"\"' : ''} value="${value}" />--}%
  %{--TODO list only servers for current user--}%
  %{--<g:select class="form-control" name="${property}" from="${flowgate.AnalysisServer.findAllByUser(sss?.currentUser)}" required="" value="${value}" />--}%
  <g:select name="${property}.id" from="${flowgate.AnalysisServer.list()}" required="" value="${((value =~ /\:(.*)/)[0][1]).toLong()}" optionKey="id" optionValue="name"/>
  <label>${label} ${required ? '*' : ''}</label>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(elems);
  });
</script>
