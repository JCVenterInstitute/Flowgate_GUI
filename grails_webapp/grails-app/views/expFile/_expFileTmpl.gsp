<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<li>
  <div class="collapsible-header"><i class="material-icons">expand_more</i>${expFile?.title}</div>

  <div class="collapsible-body">
    <g:each in="${expFile?.metaDatas.sort { it.dispOrder }}" var="eMeta">
      <div class="chip">${eMeta.mdVal}</div>
    </g:each>
  </div>
</li>
%{--<g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
  <div class="panel panel-default" >
    <div class="panel-heading">
      <div class="row" style="padding: 0;">
        <div class="col-sm-10" style="padding: 0;">
          ${expFile?.title}
        </div>
        <div class="col-sm-2" style="padding: 0;" >
          <div class="pull-right">
            <div class="btn-group">
              <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExpFileDelete" >
                <div class="btn-toolbar btn btn-default" onclick="expFileDelete(${expFile?.id})" >
                  <i class="glyphicon glyphicon-trash"></i>
                </div>
              </sec:ifAnyGranted>
              <sec:ifNotGranted roles="ROLE_Administrator,ROLE_Admin,ROLE_ExpFileDelete" >
                <g:isOwner object="expFile" objectId="${expFile?.id}">
                  <div class="btn-toolbar btn btn-default" onclick="expFileDelete(${expFile?.id})" >
                    <i class="glyphicon glyphicon-trash"></i>
                  </div>
                </g:isOwner>
              </sec:ifNotGranted>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="panel-body ${session.expFileOpenIds?.contains(expFile?.id) ?: 'collapse'}"  >
      fcs file name
      <hr/>
      <br/>
      <g:render template="/expFile/toolBar" />
    </div>
  </div>
</g:if>
<g:else>
  <div class="panel panel-default" >
    <div class="panel-heading">
      <div class="row" style="padding: 0;" >
        <div class="col-sm-11" style="padding: 0;" >
            ${expFile?.title}
        </div>
      </div>
    </div>
    <div class="panel-body ${session.expFileOpenIds?.contains(expFile?.id) ?: 'collapse'}"  >
      ${expFile.fileName}
    </div>
  </div>
</g:else>--}%

