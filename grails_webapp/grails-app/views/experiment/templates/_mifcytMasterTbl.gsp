<%@ page import="flowgate.ExpFile; flowgate.ExperimentMetadataValue; flowgate.ExperimentMetadata" %>
%{--miDat=${ExperimentMetadata.findAllByIsMiFlowAndVisible(true, true)}--}%
%{--cats=${ExperimentMetadata.findAllByExperimentAndIsMiFlowAndVisible(experiment, true, true)*.mdCategory?.unique()?.mdCategory}--}%
<g:set var="cats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisible(experiment, true, true)*.mdCategory?.unique()}" />
<g:if test="${cats}" >
  <g:form controller="experiment" action="saveMiFlowData" >
    <g:hiddenField name="eId" value="${experiment?.id}" />
    <g:hiddenField name="id" value="${experiment?.id}" />
    <div class="pull-right">
      <g:actionSubmit class="btn btn-success" value="Save and Return" action="saveMiFlowData" />
    </div>
    <br/>
    <br/>
    <br/>
  <div class="scroll-wrapper" style="overflow-x: scroll; overflow-y:hidden;">
    <div class="scroll-top" style="height: 1px;"></div>
    <div id="wholeTbl" style="overflow-y: auto;margin-bottom: 100px;">
      <table id="mifcyt-annotation-table" cellspacing="0" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%" >
        <tbody id="mifctyTbl">
          <g:each in="${cats}" var="miFlowCat" >
          <tr>
            <td style="width: 20%">${miFlowCat?.mdCategory}</td>
            <g:set var="mFDats" value="${ExperimentMetadata?.findAllByExperimentAndIsMiFlowAndVisibleAndMdCategory(experiment, true, true, miFlowCat)?.sort {it.dispOrder}}"/>
            <td style="width: 80%">
            <g:each in="${mFDats}" var="miFlowDat" >
              <g:set var="mValue" value="${ExperimentMetadataValue.findByExpMetaData(ExperimentMetadata.get(miFlowDat.id))?.mdValue}"/>
              <p><span class="col-md-3">${miFlowDat?.mdKey}</span>&nbsp;<g:textField style="width: 70%" name="expMetaData-${miFlowDat.id}" value="${mValue}" /></p>
            </g:each>
            </td>
          </tr>
          </g:each>
        </tbody>
      </table>
    </div>
  </div>
  </g:form>
</g:if>
<g:else>
  <g:link class="btn btn-success" controller="experiment" action="index" params="[eId: experiment?.id]">
      <g:message code="submitBtn.label" default="Return"/>
  </g:link>
  <div class="text-center alert alert-warning">Missing MiFlowCyt data!</div>
</g:else>
