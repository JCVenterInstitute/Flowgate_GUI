<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="wTreeSideBar" />
        <g:set var="entityName" value="${message(code: 'experiment.label', default: 'Experiment')}" />
        <title><g:message code="default.list.label" args="[entityName]" /></title>
    </head>
    <body>
        %{--
        <a href="#list-experiment" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
        <div class="nav" role="navigation">
            <ul>
                <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
                <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
            </ul>
        </div>
        --}%
        <div id="list-experiment" class="content scaffold-list" role="main">
            <h1><g:message code="default.list.label" args="[entityName]" /></h1>
            <g:if test="${flash.message}">
                <div class="message" role="status">${flash.message}</div>
            </g:if>
            <f:table collection="${experimentList}" />

            <div class="pagination">
                <g:paginate total="${experimentCount ?: 0}" />
            </div>
        </div>
        <div id="testButton" class="btn btn-primary" onclick="testButtonClick()">test click</div>
        <script type="text/javascript">
//            $(document).ready(function () {
                function testButtonClick() {
                    $.ajax({
                        url: "${createLink(controller: 'experiment', action: 'ajaxTestButtonClick')}",
                        dataType: 'json',
                        type:"get",
                        data: {},
                        success: function(data) {
                            console.log(data.success);
                            console.log(data.message);
                            alert(data.message);
                        },
                        error: function(request, status, error){
                            alert(error)
                        },
                        complete: function () {
                            console.log('ajax completed');

                        }

                    });
                };
//            });
        </script>
    </body>
</html>