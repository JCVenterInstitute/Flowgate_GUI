<%@ page import="flowgate.Module" %>
<div class="modal-content">
  <h4>Available Modules for ${server.name}</h4>
  <ul class="collection">
    <g:each in="${modules}" var="module" status="i">
      <li class="collection-item avatar" id="module-${i}">
        <i class="material-icons circle">folder</i>
        <span class="title"><strong>${module.name}</strong></span>
        <g:if test="${module.lsid}">
          <span class="lsid" style="display: none;">${module.lsid}</span>

          <p>${module.description}</p>
        </g:if>
        <g:else>
          <span class="lsid" style="display: none;">${module.id}</span>

          <p>Number of steps for this workflow: <b>${module.number_of_steps}</b></p>
        </g:else>
        <g:if test="${Module.findByNameAndServer(module.lsid, server) == null}">
          <a href="#!" onclick="fillFormWithSelectedModule('module-' +${i});" class="secondary-content tooltipped" data-tooltip="Select and configure this module"
             data-position="left">
            <i class="material-icons">add</i>
          </a>
        </g:if>
        <g:else>
          <a href="#!" class="secondary-content tooltipped text-red" data-tooltip="This module already exists in FlowGate" data-position="left" style="cursor: not-allowed">
            <i class="material-icons red-text">clear</i>
          </a>
        </g:else>
      </li>
    </g:each>
  </ul>
</div>

<div class="modal-footer">
  <a href="#!" class="modal-close waves-effect waves-green btn-flat">Close</a>
</div>

<script>
  function fillFormWithSelectedModule(module) {
    $('input[name="title"]').val($("#" + module + " > .title > strong").text());
    $('input[name="name"]').val($("#" + module + " > .lsid").text());
    $('textarea[name="descript"]').text($("#" + module + " > p").html());

    $('label[for="title"], label[for="name"], label[for="descript"]').addClass("active");
    $("#modules-list").modal('close');
  }
</script>
