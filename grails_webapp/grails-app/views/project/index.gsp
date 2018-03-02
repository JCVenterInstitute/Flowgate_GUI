<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="wTreeSideBar" />
        <g:set var="entityName" value="${message(code: 'project.label', default: 'Project')}" />
        <title><g:message code="default.projectView.label" default="Project" args="[entityName]" /></title>
        <asset:javascript src="jquery-2.2.0.min.js"/>
    </head>
    <body>
        <g:render template="/shared/nav" />
        <content tag="topBtnBar">
            <div id="topBtnBar">
                <g:render template="templates/indexTopBtnBar" model="[project: this.project]" />
            </div>
            <g:render template="/shared/errorsMsgs" model="[bean: this.project]" />
        </content>
        <content tag="treeView">
            <div id="projTree">
                %{--TODO rename project to projectList--}%
                <g:render template="/shared/treeView" model="[projectList: projectList, experimentList: experimentList]" />
            </div>
        </content>
        <content tag="pgContent">
            <div id="pageContent">
                <g:render template="templates/indexTmpl" model="[experimentList: experimentList]"/>
            </div>
        </content>
    </body>
</html>