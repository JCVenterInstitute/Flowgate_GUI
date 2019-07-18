<!DOCTYPE html>
<html>
<head>
  <meta name="layout" content="main"/>
  <g:set var="entityName" value="${message(code: 'expFile.label', default: 'Experiment File')}"/>
  <title><g:message code="default.expFile.annotation.label" default="Annotation" args="[entityName]"/></title>
  <style>
  #toAnno, #toFcs {
    background-color: rgba(122, 122, 122, 0.27);
  }
  </style>
</head>

<body>

%{--
experiment=${experiment} id=${experiment?.id} ${experiment == null}
project=${experiment?.project}
--}%

<g:if test="${experiment}">
  <div class="container">
    <h4>${experiment?.title} - Annotation</h4>
  <g:form controller="expFile" action="doAnnotate" params="[experiment1: experiment]" id="${experiment.id}">
    <g:hiddenField name="expId" value="${experiment?.id}"/>
    <g:hiddenField name="projId" value="${experiment?.project?.id}"/>
    <div class="row" style="max-width: 100%">
      <div class="col-sm-2" style="padding-right: 0">
        %{--<content tag="fcsPanel">--}%
        <div id="metaData">
          <g:render template="annotationTmpl/mdFilterPanel" model="[experiment: experiment]"/>
        </div>
        %{--</content>--}%
      </div>

      <div class="col-sm-2" style="padding-left: 0">
        %{--<content tag="fcsPanel">--}%
        <div id="fcsPanel">
          <g:render template="annotationTmpl/fcsFilePanel" model="[experiment: experiment]"/>
        </div>
        %{--</content>--}%
      </div>

      <div class="col-sm-1">
        <div id="annotateBtnPnl" style="padding-top: 150px">
          %{--<p><div id="toAnno" class="btn btn-default" onclick="alert('not implemented yet!');" style="width: 100%" ><i class="fa fa-caret-right" ></i></div></p>--}%
          <p>&nbsp</p>

          <p><button id="toFcs" type="submit" value="action_left" class="btn btn-default" style="width: 100%"><i class="fa fa-caret-left"></i></button></p>
        </div>
      </div>

      <div class="col-sm-4">
        %{--<content tag="pgContent">--}%
        <div id="annotation">
          <g:render template="annotationTmpl/annotationPanel" model="[experiment: experiment]"/>
        </div>

        <div class="pull-right" style="padding-right: 30px">
          <div class="btn btn-default"><i class="fa fa-plus" onclick="alert('not implemented yet!');"></i></div>
        </div>
        <br/>
        <br/>
        <br/>

        <div class="text-center">
          <div class="btn btn-success" onclick="alert('not implemented yet!');">Submit</div>
        </div>
        %{--</content>--}%
      </div>

      <div class="col-sm-3">
        %{--<content tag="pgRightPanel">--}%
        <br/>
        <br/>
        <br/>
        %{--<div class="text-center" style="padding-right: 20px">--}%
        <div class="text-center">
          <a class="noLinkBlack btn btn-default" href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment?.id)}"><i
              class="fa fa-table"></i>&nbsp;Overview Table</a>
        </div>
        <br/>
        <br/>
        <br/>
        %{--</content>--}%
      </div>
    </div>
  </g:form>
  </div>
</g:if>
</body>
</html>
