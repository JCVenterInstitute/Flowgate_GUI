<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}"/>
  <title><g:message code="default.list.label" args="[entityName]"/></title>
</head>

<body>
<div id="list-experiment" class="content scaffold-list" role="main">
  <h1><g:message code="default.list.label" args="[entityName]"/></h1>
  <f:table collection="${experimentList}"/>

  <div class="pagination">
    <g:paginate total="${experimentCount ?: 0}"/>
  </div>
</div>

<div id="testButton" class="btn btn-primary" onclick="testButtonClick()">test click</div>
<script type="text/javascript">
  function testButtonClick() {
    $.ajax({
      url: "${createLink(controller: 'experiment', action: 'ajaxTestButtonClick')}",
      dataType: 'json',
      type: "get",
      data: {},
      success: function (data) {
        console.log(data.success);
        console.log(data.message);
        alert(data.message);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');

      }

    });
  }
</script>
</body>
</html>