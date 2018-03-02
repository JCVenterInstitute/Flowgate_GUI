<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'analysisServer.label', default: 'AnalysisServer')}" />
        <title><g:message code="default.list.label" args="[entityName]" /></title>
    </head>
    <body>
        <a href="#list-analysisServer" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
        <div class="nav" role="navigation">
            <ul>
                <li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
                <li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
            </ul>
        </div>
        <div id="list-analysisServer" class="content scaffold-list" role="main">
            <h3 class="text-center"><g:message code="default.list.label" args="[entityName]" /></h3>
            <g:if test="${flash.message}">
                <div class="message" role="status">${flash.message}</div>
            </g:if>
            <div class="row">
                <div class="col-sm-offset-1 col-sm-10">
                    <f:table collection="${analysisServerList}" properties="['name','url','userName','userPw']" />
                    <div class="pagination">
                        <g:paginate total="${analysisServerCount ?: 0}" />
                    </div>
                </div>
            </div>
        </div>
    </body>
</html>