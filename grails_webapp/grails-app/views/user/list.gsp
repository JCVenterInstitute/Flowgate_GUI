<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'user.label', default: 'Users')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<g:if test="${newUserLst.size() > 0}">
  <h2><g:message code="default.list.label" args="[entityName]"/></h2>

  <table class="highlight responsive-table">
    <thead>
    <tr>
      <g:sortableColumn property="username" title="Username"/>
      <g:sortableColumn property="email" title="Email"/>
      <g:sortableColumn property="enabled" title="Active"/>
    </tr>
    </thead>
    <tbody>
    <g:each in="${newUserLst}" var="nu" status="i">
      <tr>
        <td>
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
        <td>
          <f:display bean="${nu}"
                     property="${'email'}"
                     displayStyle="${displayStyle ?: 'table'}"/>
        </td>
        <td>
          <div id="nu-act-${nu.id}" %{--onclick="activate(${nu.id})"--}%><g:render template="activateField" model="[user: nu]"/></div>
        </td>
      </tr>
    </g:each>
    </tbody>
  </table>
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
  <p>No new users</p>
</g:else>

<div class="fixed-action-btn">
  <g:link class="btn-floating btn-large waves-effect waves-light tooltipped" action="create" data-tooltip="Add a new user" data-position="left">
    <i class="material-icons">add</i>
  </g:link>
</div>

<script type="text/javascript">
  $(document).ready(function () {
    $("nav .nav-wrapper li").removeClass("active");
    $("nav #nav-users").addClass("active");
  });
  document.addEventListener('DOMContentLoaded', function () {
    var tooltipElems = document.querySelectorAll('.tooltipped');
    M.Tooltip.init(tooltipElems);
  });

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