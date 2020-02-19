<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div id="wholeTbl" style="overflow-y: auto;margin-bottom: 100px;">
  <table id="annotation-table" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
    <thead>
    <tr >
      <g:sortableColumn property="expFile" title="${message(code: 'expFile.label', default: 'FCS File Name')}">
        <p><br/></p>
      </g:sortableColumn>
      <g:each in="${experiment.expMetadatas.findAll { it?.mdCategory == category }.sort { it.dispOrder }}" var="eMeta">
        <g:if test="${eMeta.visible}">
          <th class="sortable">
            <p class="text-center" >${eMeta.mdKey}</p>
            <p class="text-center">
              <g:set var="filterAction" value="${eMeta.dispOnFilter ? 'HideFromFilter' : 'ShowOnFilter'}"/>
              <g:set var="colActions" value="['Edit', 'HideColumn', filterAction, 'Delete']"/>
              <div class="input-group input-group-sm">
                <span class="input-group-addon">Action</span>
                <g:select class="form-control" name="colAction" from="${colActions}" noSelection="['': '']" style="min-width: 30px"
                          optionValue="${{ action -> g.message(code: 'annotationPage.action.' + action + '.label') }}" onchange="eMetaActionChange(${eMeta.id}, this.value);"/>
              </div>
            </p>
            <p class="text-center">
              <g:if test="${eMeta.mdVals.size() > 1}">
                <div class="input-group input-group-sm">
                  <span class="input-group-addon">Candidate Values</span>
                  <g:select style="width:auto;" class="form-control" id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort { it.dispOrder }}" optionKey="id"
                            optionValue="mdValue" value="" onchange="eMetaValueChange(${eMeta.id}, this.value);"/>
                </div>
              </g:if>
              <g:else>
                <g:hiddenField id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" value="${eMeta?.mdVals.id.join(',')}"/>
                ${eMeta.mdVals.mdValue.join(',')}
              </g:else>
            </p>
          </th>
        </g:if>
      </g:each>
      <th class="text-center">
        <br/>

        <div class="${experiment.expMetadatas.findAll { it?.mdCategory == category }.visible.toString().contains('false') ? '' : 'hidden'} btn btn-default"
             onclick="showAllHidden(${experiment.id}, '${category?.id}');">Show All Hidden Attributes</div>
        <br/>
        <br/>

        <div style="padding-left:30px;">
          <g:if test="${category?.mdCategory != 'Reagents'}">
            <div class="" onclick="addColClick(${experiment?.id}, '${category}');"><i title="add column" class="fa fa-plus fa-2x"></i></div>
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
    <g:render template="annotationTmpl/tablTmpl" model="[category: category]"/>
    </tbody>
  </table>
</div>
