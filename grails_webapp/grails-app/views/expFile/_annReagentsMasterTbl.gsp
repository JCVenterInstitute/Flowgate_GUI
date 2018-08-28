<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<table style="margin-top:0;" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%" >
  <thead>
    <tr>
      <g:sortableColumn property="expFile" title="${message(code: 'expFile.label', default: 'Exp. File')}" >
         <p><br/></p>
      </g:sortableColumn>
      %{--TODO retrieve channels of all fcs-files as list --}%
      %{--<g:each in="${['FL1-A','FL2-A','FL3-A','FL4-A','FL5-A','FL6-A','FL7-A'] }" var="eMeta">--}%
      %{--<g:each in="${experiment.expMetadatas.findAll { it.mdCategory == 'Reagents' }.sort { it.dispOrder }}" var="eMeta">--}%
      <g:each in="${experiment.expMetadatas.find{ it.mdKey == 'Channel' }?.mdVals?.sort{it.dispOrder}}" var="channel">
      %{--<g:each in="${metaDatas.mdVals}" var="eMeta">--}%
        <th class="sortable">
          <p class="text-center">action
              <g:select name="colAction" from="['Sort','Hide','Filter', 'Select All', 'Select None']" />
          </p>
          %{--<p class="text-center" >${eMeta.mdKey}</p>--}%
          <p class="text-center" >
            %{--<g:if test="${eMeta.mdVals.size()>1}" >--}%
                %{--<g:select id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort{it.dispOrder}}" optionKey="id" optionValue="mdValue" value="" onchange="eMetaValueChange(${eMeta.id}, this.value);"/>--}%
            %{--</g:if>--}%
            %{--<g:else>--}%
                %{--<g:hiddenField id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" value="${eMeta?.mdVals.id.join(',')}" />--}%
                %{--${eMeta.mdVals.mdValue.join(',')}--}%
            %{--</g:else>--}%
            ${channel.mdValue}
            &nbsp;<i style="font-size: xx-small;cursor: pointer" class="fa fa-pencil" onclick="alert('not yet implemented');"></i>
          </p>
        </th>
      </g:each>
      <th class="text-center">
        <br/>
        <br/>
        <br/>
        <i title="add column" class="fa fa-plus fa-2x" onclick="alert('not yet implemented');"></i>
      </th>
    </tr>
  </thead>
  <tbody id="fcsTbl">
    <g:render template="annotationTmpl/reagentsTablTmpl" model="[]" />
  </tbody>
</table>
%{--
<div class="pagination">
  <g:paginate total="${plateCount ?: 0}" />
</div>
--}%