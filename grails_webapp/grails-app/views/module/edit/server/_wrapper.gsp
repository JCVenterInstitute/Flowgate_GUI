%{--<g:set var="sss" bean="springSecurityService"/>--}%
<%@ page import="flowgate.AnalysisServer" defaultCodec="html" %>
<div class="form-group">
  <div class="fieldcontain ${required ? 'required' : ''}">
    <label for="${property}" style="width: 100%;text-align: left">${label} ${required ? '*' : ''}</label>
    %{--<input type="text" class="form-control" name="${property}" ${required ? 'required=\"\"' : ''} value="${value}" />--}%
    %{--TODO list only servers for current user--}%
    %{--<g:select class="form-control" name="${property}" from="${flowgate.AnalysisServer.findAllByUser(sss?.currentUser)}" required="" value="${value}" />--}%
    <g:select class="form-control" name="${property}.id" from="${flowgate.AnalysisServer.list()}" required="" value="${((value =~ /\:(.*)/)[0][1]).toLong()}" optionKey="id" optionValue="name" />
  </div>
</div>
