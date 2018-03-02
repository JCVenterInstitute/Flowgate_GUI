<!DOCTYPE html>
<html>
    <head>
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}" />
        <title><g:message code="default.expFile.annotation.label" default="Annotation" args="[entityName]" /></title>
        <asset:javascript src="jquery-2.2.0.min.js"/>
        <style>
            .noLinkBlack {
                color: black;
                text-decoration: none;
            }
            /* table with horizontal scroll */
            /*
            table {
                !*width: 100%;*!
                display: block;
                overflow-x: auto;
            }
            */
        </style>
    </head>
    <body>
        <g:render template="/shared/nav" />

        <h2 class="text-center"><g:message code="annotation.table.label" default="Annotation Table" /></h2>
        <br/>
        <br/>
        <g:render template="annMasterTbl" />
        <br/>
        <br/>
        %{--
        <div class="text-center">
            <div class="btn btn-success" onclick="alert('not implemented yet!');" >Submit</div>
        </div>
        --}%
    </body>
</html>