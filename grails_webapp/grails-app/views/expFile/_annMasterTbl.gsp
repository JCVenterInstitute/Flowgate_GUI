<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
%{--<g:set var="colActions" value="['Sort','Hide','Filter', 'Select All', 'Select None', 'Delete']" />--}%
<g:set var="colActions" value="['Edit','Hide', 'Delete']" />
<div id="wholeTbl" >
  <table style="margin-top:0;" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%" >
    <thead>
      <tr>
        <g:sortableColumn property="expFile" title="${message(code: 'expFile.label', default: 'Exp. File')}" >
           <p><br/></p>
        </g:sortableColumn>
        <g:each in="${experiment.expMetadatas.findAll{it.mdCategory== category}.sort{it.dispOrder} }" var="eMeta">
          <g:if test="${eMeta.visible}" >
          <th class="sortable">
            <p class="text-center">action
                <g:select name="colAction" from="${colActions}" noSelection="['':'']" onchange="eMetaActionChange(${eMeta.id}, this.value);" />
            </p>
            <p class="text-center" >${eMeta.mdKey}</p>
            <p class="text-center" >
              <g:if test="${eMeta.mdVals.size()>1}" >
                  <g:select id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort{it.dispOrder}}" optionKey="id" optionValue="mdValue" value="" onchange="eMetaValueChange(${eMeta.id}, this.value);"/>
              </g:if>
              <g:else>
                  <g:hiddenField id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" value="${eMeta?.mdVals.id.join(',')}" />
                  ${eMeta.mdVals.mdValue.join(',')}
              </g:else>
            </p>
            %{--<div id="colEditModal">--}%
              %{--<g:render template="annotationTmpl/colEditModal" model="[experiment: experiment, eMeta: eMeta]"/>--}%
            %{--</div>--}%
          </th>
          </g:if>
        </g:each>
        <th class="text-center">
          <br/>
            <div class="btn btn-default" onclick="showAllHidden(${experiment.id}, '${category}');">Show Hidden Cols</div>
          <br/>
          <br/>
          <div  style="padding-left:30px;" >
            <g:if test="${category in ['Demographics','Visit','Stimulation']}">
              %{--<div class="" data-toggle="modal" data-target="#addColForm"><i title="add column" class="fa fa-plus fa-2x" ></i></div>--}%
              <div class="" onclick="addColClick(${experiment?.id}, '${category}');"><i title="add column" class="fa fa-plus fa-2x" ></i></div>
            </g:if>
            %{--<g:else>--}%
               %{-- add panel reagent --}%
            %{--</g:else>--}%
          </div>
          <br/>
        </th>
      </tr>
    </thead>
    <tbody id="fcsTbl">
      <g:render template="annotationTmpl/tablTmpl" model="[category: category]" />
    </tbody>
  </table>
</div>
%{--<div id="colCreateModal">--}%
  %{--<g:render template="annotationTmpl/colCreateModal" model="[experiment: experiment]"/>--}%
%{--</div>--}%
%{--
<div class="pagination">
  <g:paginate total="${plateCount ?: 0}" />
</div>
--}%