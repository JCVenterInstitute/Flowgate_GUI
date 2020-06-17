<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
<div id="wholeTbl" style="overflow-y: auto;">
  <table id="annotation-table" cellspacing="0" class="highlight responsive-table small" width="100%">
    <thead>
    <tr>
      <g:sortableColumn property="expFile" title="${message(code: 'expFile.label', default: 'FCS File Name')}"/>
      <g:each in="${experiment.expMetadatas.findAll { it?.mdCategory == category }.sort { it.dispOrder }}" var="eMeta">
        <g:if test="${eMeta.visible}">
          <th class="sortable">
            <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
              <div onclick="toggleActions(this, ${eMeta.id})" style="cursor: pointer;">
            </sec:ifAnyGranted>
            ${eMeta.mdKey}
            <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
              <i class="material-icons tiny">expand_more</i></div>
            </sec:ifAnyGranted>
            <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
              <div class="attribute-action" id="attribute-action-${eMeta.id}" style="display: none;">
                <g:set var="filterAction" value="${eMeta.dispOnFilter ? 'HideFromFilter' : 'ShowOnFilter'}"/>
                <button type="button" id="Edit" class="btn-tiny waves-effect waves-light tooltipped" onclick="eMetaActionChange(${eMeta.id}, this.id);
                return false;"
                        data-tooltip="Edit attribute" data-position="bottom">
                  <i class="material-icons">edit</i>
                </button>
                <button type="button" id="HideColumn" class="btn-tiny waves-effect waves-light tooltipped" onclick="eMetaActionChange(${eMeta.id}, this.id);
                return false;"
                        data-tooltip="Hide attribute" data-position="bottom">
                  <i class="material-icons">visibility_off</i>
                </button>
                <button type="button" id="${filterAction}" class="btn-tiny waves-effect waves-light tooltipped" onclick="eMetaActionChange(${eMeta.id}, this.id);
                return false;"
                        data-tooltip="<g:if test="${filterAction == 'HideFromFilter'}">Hide</g:if><g:else>Show</g:else> attribute in Dataset" data-position="bottom">
                  <i class="material-icons"><g:if test="${filterAction == 'HideFromFilter'}">grid_off</g:if><g:else>grid_on</g:else></i>
                </button>
                <button type="button" id="Delete" class="btn-tiny waves-effect waves-light tooltipped" onclick="eMetaActionChange(${eMeta.id}, this.id);
                return false;"
                        data-tooltip="Delete attribute" data-position="bottom">
                  <i class="material-icons">delete</i>
                </button>
              </div>
            </sec:ifAnyGranted>

            <g:if test="${eMeta.mdVals.size() > 1}">
              <div class="input-field">
                <g:select id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" from="${eMeta.mdVals.sort { it.dispOrder }}"
                          optionKey="id"
                          optionValue="mdValue" value="" onchange="eMetaValueChange(${eMeta.id}, this.value);"/>
              </div>
            </g:if>
            <g:else>
              <div class="input-field">
                <g:hiddenField id="eMeta_${eMeta.id}.mdVal" name="eMeta_${eMeta.id}.mdVal" value="${eMeta?.mdVals.id.join(',')}"/>
                ${eMeta.mdVals.mdValue.join(',')}
              </div>
            </g:else>
          </th>
        </g:if>
      </g:each>
      <sec:ifAnyGranted roles="ROLE_Administrator,ROLE_Admin">
        <th>
          <a href="#" class="${experiment.expMetadatas.findAll { it.mdCategory == category }.visible.toString().contains('false') ? '' : 'hidden'}"
             onclick="showAllHidden(${experiment.id}, '${category?.id}');">Show All Hidden Attributes</a>
          <g:if test="${category?.mdCategory != 'Reagents'}">
            <a href="#add-new-attribute" class="modal-trigger tooltipped" data-tooltip="Add a new column" data-position="right">
              <i class="material-icons">add</i>
            </a>
          </g:if>
        </th>
      </sec:ifAnyGranted>
    </tr>
    </thead>
    <tbody id="fcsTbl">
    <g:render template="annotationTmpl/tablTmpl" model="[category: category]"/>
    </tbody>
  </table>
</div>
