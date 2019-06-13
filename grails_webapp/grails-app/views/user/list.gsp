<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'Users')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<div class="nav" role="navigation"></div>
<div id="list-user" class="content scaffold-list" role="main">
  <g:if test="${newUserLst.size() > 0}">
    <div class="row">
      <div class="col-sm-offset-1 col-sm-10">
        <h1 class="page-header"><g:message code="default.list.label" args="[entityName]"/></h1>
        <g:if test="${flash.message}">
          <div class="message" role="status">${flash.message}</div>
        </g:if>
        <table class="table table-bordered table-responsive table-striped table-hover dataTable">
          <thead>
          <tr>
            <g:sortableColumn class="text-center" property="username" title="Username"/>
            <g:sortableColumn class="text-center" property="email" title="Email"/>
            <g:sortableColumn class="text-center" property="enabled" title="Active"/>
          </tr>
          </thead>
          <tbody>
          <g:each in="${newUserLst}" var="nu" status="i">
            <tr>
              <td class="text-center">
                <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
                %{--  Show User method  --}%
                  <g:link method="GET" resource="${nu}" action="edit">
                    <f:display bean="${nu}"
                               property="username"
                               displayStyle="${displayStyle ?: 'table'}"/>
                  </g:link>
                </sec:ifAnyGranted>
                <sec:ifNotGranted roles="ROLE_Administrator,ROLE_Admin">
                  <f:display bean="${nu}"
                             property="${'username'}"
                             displayStyle="${displayStyle ?: 'table'}"/>
                </sec:ifNotGranted>
              </td>
              <td class="text-center">
                <f:display bean="${nu}"
                           property="${'email'}"
                           displayStyle="${displayStyle ?: 'table'}"/>
              </td>
              <td class="text-center">
                <div id="nu-act-${nu.id}" %{--onclick="activate(${nu.id})"--}%><g:render template="activateField" model="[user: nu]"/></div>
              </td>
            </tr>
          </g:each>
          </tbody>
        </table>
      </div>
    </div>
    <g:if test="${newUserLst.size() > 10}">
      <div class="row">
        <div class="col-sm-offset-1 col-sm-10">
          <div class="pagination">
            <g:paginate total="${newUserLstCount ?: 0}"/>
          </div>
        </div>
      </div>
    </g:if>
  </g:if>
  <g:else>
    <h3 class="text-center">No new users</h3>
  </g:else>
</div>
<script type="text/javascript">
  function activate(uId) {
    $.ajax({
      url: "${createLink(controller: 'user', action: 'axActivateUser')}",
      dataType: "json",
      type: "get",
      data: {uId: uId},
      success: function (data) {
        $("#nu-act-" + uId).html(data.activatedField);
      },
      error: function (request, status, error) {
        alert(error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }
</script>
</body>
</html>