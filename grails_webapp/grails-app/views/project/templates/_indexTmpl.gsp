<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>
<g:if test="${project}">
  <f:with bean="project">
    <h2><f:display property="title"/></h2>

    <div class="row">
      <div class="col s12">
        <ul class="tabs">
          <li class="tab col s3"><a href="#description">Project Description</a></li>
          <li class="tab col s3"><a class="active" href="#experiments">Experiments</a></li>
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

<script>
  document.addEventListener('DOMContentLoaded', function () {
    var elems = document.querySelectorAll('.tabs');
    M.Tabs.init(elems);
  });
</script>
