<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${experiment?.id == session?.experimentEditModeId?.toLong()}">
  <div class="panel panel-default" >
    <div class="panel-heading">
      <div class="row" style="padding: 0;">
        <div class="col-sm-10" style="padding: 0;">
          <input class="form-control" type="text" value="${expFile?.title}" onchange="expFileTitleChange(${expFile?.id})" (mouseup)="$event.target.select()" />
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
              <div class="btn-toolbar btn btn-default" onclick="toggleExpFileOpen(${experiment?.id},${expFile?.id})">
                <i class="glyphicon ${session.expFileOpenIds?.contains(expFile?.id) ? 'glyphicon-chevron-up' : 'glyphicon-chevron-down' }" ></i>
              </div>
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
          %{--<div class="form-control">--}%
            ${expFile?.title}
          %{--</div>--}%
        </div>
        <div class="col-sm-1" style="padding: 0;" >
          <div class="pull-right">
            <div class="btn-group">
              <div class="btn-toolbar btn btn-default" onclick="toggleExpFileOpen(${experiment?.id},${expFile?.id})">
                <i class="glyphicon ${session.expFileOpenIds?.contains(expFile?.id) ? 'glyphicon-chevron-up' : 'glyphicon-chevron-down' }" ></i>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="panel-body ${session.expFileOpenIds?.contains(expFile?.id) ?: 'collapse'}"  >
      ${expFile.fileName}&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<i class="fa fa-list-alt"></i>
      &nbsp;&nbsp;&nbsp;&nbsp;<i class="fa fa-table"></i>
      &nbsp;&nbsp;&nbsp;&nbsp;<i class="fa fa-picture-o"></i>
      &nbsp;&nbsp;&nbsp;&nbsp;<i class="glyphicon glyphicon-apple"></i>
      %{--&nbsp;&nbsp;&nbsp;--- no owner no buttons--}%
    </div>
  </div>
</g:else>

