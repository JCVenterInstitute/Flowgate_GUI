<%@ page import="flowgate.ExpFile; flowgate.Experiment; flowgate.User; flowgate.ExperimentUser" %>
<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>

<div class="navigation nav-wrapper">
  <div class="col s12">
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="#!" class="breadcrumb dark">${experiment?.title}</a>
  </div>
</div>

<h2>${experiment?.title}</h2>

<ul class="tabs">
  <li class="tab col s3"><a class="active" href="#description">Experiment Description</a></li>
  <li class="tab col s3"><a href="#fcs-files">FCS Files</a></li>
  <li class="tab col s3"><a href="#manage-users">Users</a></li>
</ul>

<div id="description">
  <div class="row">
    <div class="input-field col s12">
      <p>${experiment?.description}</p>
    </div>
  </div>
</div>

<div id="fcs-files">
  <div class="row">
    <div class="input-field col s12">
      <sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
        <a class="btn waves-effect waves-light" href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">Upload FCS File</a>
      </sec:ifAnyGranted>
      <sec:ifNotGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
        <g:if test="${utilsService.isAffil('experiment', experiment?.id)}">
          <a class="btn waves-effect waves-light" href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">Upload FCS File</a>
        </g:if>
      </sec:ifNotGranted>
      <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
        <a class="btn waves-effect waves-light" href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment?.id)}">FCS File Annotation</a>
      </g:isOwnerOrRoles>
    </div>

    <div class="input-field col s12">
    <p>
      <g:if test="${experiment && ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment?.id?.toLong(), true), true)}">
        This experiment currently contains <b>${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true).size()}</b> FCS files.

        <ul class="collapsible">
          <g:each in="${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true)}" var="expFile">
            <g:render template="/expFile/expFileTmpl" model="[experiment: experiment, expFile: expFile]"/>
          </g:each>
        </ul>
      %{--
      <g:each in="${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true)}" var="expFile" >
        <div id="expFile-${expFile?.id}">
          <g:render template="/expFile/expFileTmpl" model="[experiment: experiment, expFile: expFile]"/>
        </div>
      </g:each>
      --}%
      </g:if>
      <g:else>
        This experiment doesn't have any FCS file.
      </g:else>
    </p>
    </div>
  </div>
</div>

<div id="manage-users">
  <div class="row">
    <g:form name="manageUsers" controller="experiment" action="manageUsers" id="${experiment?.id}">
      <div class="col s12">
        <div class="row">
          <div class="input-field col s6">
            <g:select id="owners-${experiment?.id}"
                      name="owners"
                      value="${ExperimentUser?.findAllByExperimentAndExpRole(experiment, 'owner')*.user*.id}"
                      from="${User.list()}"
                      optionKey="id"
                      optionValue="username"
                      multiple=""/>
            <label>Owner(s)</label>
          </div>
        </div>
      </div>

      <div class="col s12">
        <div class="row">
          <div class="input-field col s6">
            <g:select id="members-${experiment?.id}"
                      name="members"
                      value="${ExperimentUser?.findAllByExperimentAndExpRole(experiment, 'member')*.user*.id}"
                      from="${User.list()}"
                      optionKey="id"
                      optionValue="username"
                      multiple=""/>
            <label>Member(s)</label>
          </div>
        </div>
      </div>

      <div class="input-field col s12">
        <button type="submit" class="btn waves-effect waves-light">Save</button>
      </div>
    </g:form>
  </div>
</div>

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var tabElems = document.querySelectorAll('.tabs');
    M.Tabs.init(tabElems);

    var selectElems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(selectElems);

// <<<<<<< HEAD
    var elems = document.querySelectorAll('.collapsible');
    var instances = M.Collapsible.init(elems, {
      onOpenStart: function (e) {
        e.querySelector("i").textContent = "expand_less";
      },
      onCloseEnd: function (e) {
        e.querySelector("i").textContent = "expand_more";
      },
    });
  });
</script>
%{--=======--}%
%{--
--}%
%{--</g:if>--}%
%{--</g:form>--}%
%{--<g:render template="/shared/manageUsers" model="[objectType: 'Experiment', object: experiment]"/>--}%
%{-->>>>>>> dev-int--}%
