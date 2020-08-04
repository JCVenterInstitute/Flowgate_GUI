<%@ page import="flowgate.User; flowgate.ProjectUser" %>
<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${project}">
  <f:with bean="project">
    <h2><f:display property="title"/></h2>

    <div class="row">
      <div class="col s12">
        <ul class="tabs">
          <li class="tab col s3"><a href="#description">Project Description</a></li>
          <li class="tab col s3"><a class="active" href="#experiments">Experiments</a></li>
          <li class="tab col s3"><a href="#manage-users">Project Users</a></li>
        </ul>
      </div>

      <div id="description" class="col s12">
        <div class="row">
          <div class="input-field col s12">
            <p>${project?.description}</p>
          </div>
        </div>
      </div>
    </div>
  </f:with>
</g:if>

<div id="experiments">
  <div id="expContent">
    <g:render template="templates/expListTmpl" model="[experimentList: experimentList]"/>
  </div>
</div>

<div id="manage-users">
  <div class="row">
    <g:form name="manageUsers" controller="project" action="manageUsers" id="${project?.id}">
      <div class="col s12">
        <div class="row">
          <div class="input-field col s6">
            <g:select id="owners-${project?.id}"
                      name="owners"
                      value="${ProjectUser?.findAllByProjectAndProjRole(project, 'owner')*.user*.id}"
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
            <g:select id="members-${project?.id}"
                      name="members"
                      value="${ProjectUser?.findAllByProjectAndProjRole(project, 'member')*.user*.id}"
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
    var elems = document.querySelectorAll('.tabs');
    M.Tabs.init(elems);

    var selectElems = document.querySelectorAll('select');
    var instances = M.FormSelect.init(selectElems);
  });
</script>
