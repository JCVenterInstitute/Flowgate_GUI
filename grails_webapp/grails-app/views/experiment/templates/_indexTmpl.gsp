<%@ page import="flowgate.ExpFile; flowgate.Experiment; flowgate.User; flowgate.ExperimentUser" %>
<% def utilsService = grailsApplication.mainContext.getBean("utilsService") %>

<style>
.active > .collapsible-header > i {
  -ms-transform: rotate(180deg); /* IE 9 */
  -webkit-transform: rotate(180deg); /* Chrome, Safari, Opera */
  transform: rotate(180deg);
}
</style>

<div class="navigation nav-wrapper">
  <div class="col s12">
    <span class="breadcrumb dark" style="cursor: pointer;"><i id="breadcrumbs_unfold" class="material-icons" style="margin-left: -15px;">unfold_more</i></span>
    <a href="${createLink(controller: 'project', action: 'index', params: [pId: experiment?.project?.id])}" class="breadcrumb dark tooltipped" data-position="bottom"
       data-tooltip="${experiment?.project?.title}">Project</a>
    <a href="#!" class="breadcrumb dark">${experiment?.title}</a>
  </div>
</div>

<h2>${experiment?.title}</h2>

<ul class="tabs">
  <li class="tab col s3"><a class="active" href="#description">Experiment Description</a></li>
  <li class="tab col s3"><a href="#fcs-files">FCS Files</a></li>
  <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" parentObject="project" parentObjectId="${experiment?.project?.id}" roles="ROLE_Administrator,ROLE_Admin">
    <li class="tab col s3"><a href="#manage-users">Experiment Users</a></li>
  </g:isOwnerOrRoles>
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
      <g:if test="${experiment && ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment?.id?.toLong(), true), true)}">
        <p>This experiment currently contains <b id="fileCount">${ExpFile?.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment.id.toLong(), true), true).size()}</b> FCS file(s).
        <sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
          You can <a href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">upload FCS File(s)</a>
        </sec:ifAnyGranted>
        <sec:ifNotGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
          <g:if test="${utilsService.isAffil('experiment', experiment?.id)}">
            You can <a href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">upload FCS File(s)</a>
          </g:if>
        </sec:ifNotGranted>
        <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
          or <a href="${g.createLink(controller: 'expFile', action: 'annotationTbl', id: experiment?.id)}">annotate existing FCS file(s)</a>
        </g:isOwnerOrRoles>
        <g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" roles="ROLE_Administrator,ROLE_Admin,ROLE_User">
          or <a href="${g.createLink(controller: 'experiment', action: 'miFcytTbl', id: experiment?.id)}">annotate experiment with MIFlowCyt data</a>
        </g:isOwnerOrRoles>

        </p>
          <!-- Switch to select files to be deleted -->

        <div class="switch">
          <label>
            Enable file selection to delete:
            <input type="checkbox" id="switch-file-selection">
            <span class="lever"></span>
          </label>
          <label style="display: none;" class="checkbox-label">
            <input type="checkbox" id="select-all-file"/>
            <span>Select All</span>
          </label>
        </div>

        <p>
          <a class="checkbox-label" href="#!" style="display: none;" onclick="openDeleteFileModal()">Delete selected files</a>
        </p>

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
        <sec:ifAnyGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
          You can <a href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">upload FCS File(s)</a>.
        </sec:ifAnyGranted>
        <sec:ifNotGranted roles="ROLE_SuperAdmin,ROLE_Administrator,ROLE_Admin,ROLE_User,ROLE_ExperimentEdit">
          <g:if test="${utilsService.isAffil('experiment', experiment?.id)}">
            You can <a href="${g.createLink(controller: 'expFile', action: 'expFileCreate', params: [eId: experiment?.id])}">upload FCS File(s)</a>.
          </g:if>
        </sec:ifNotGranted>
      </g:else>
    </div>
  </div>
</div>

<div id="delete-files-modal" class="modal">
  <div class="modal-content">
    <h4>Confirm to delete selected experiment files?</h4>
  </div>

  <div class="modal-footer">
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <button type="button" class="modal-close waves-effect waves-green btn-flat" id="confirm-file-deletion">Confirm</button>
  </div>
</div>

<div id="fcs-info-modal" class="modal" style="width: 75% !important;">
  <div class="modal-content"></div>

  <div class="modal-footer">
    <div class="col s12">
      <label>
        <input type="checkbox" class="filled-in" checked="checked" id="updateData">
        <span>Update default values</span>
      </label>
    </div>
    <div id="gate-drawing-error" class="col s4 offset-s8" style="display: none"><span class="red-text">Please fix error(s)!</span></div>
    <a href="#!" class="modal-close waves-effect waves-light btn-flat">Cancel</a>
    <button type="button" class="modal-close waves-effect waves-green btn-flat" id="confirm-gate-drawing">Confirm</button>
  </div>
</div>

<g:isOwnerOrRoles object="experiment" objectId="${experiment?.id}" parentObject="project" parentObjectId="${experiment?.project?.id}" roles="ROLE_Administrator,ROLE_Admin">
  <div id="manage-users">
    <div class="row">
      <div class="col s12">
        <p><b>Owners: </b>Experiment owners have read, edit, and delete privilege for all data and data analysis results under the experiment.</p>
        <p><b>Members: </b>Experiment members have only read privilige for the experiment.</p>
      </div>
    </div>
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
</g:isOwnerOrRoles>

<script>
  var deleteFileModalInstance;
  var fcsInfoModalInstance;
  document.addEventListener('DOMContentLoaded', function () {
    var tabElems = document.querySelectorAll('.tabs');
    M.Tabs.init(tabElems);

    var ownersSelectElems = document.querySelectorAll('#owners-${experiment?.id}');
    var membersSelectElems = document.querySelectorAll('#members-${experiment?.id}');
    M.FormSelect.init(ownersSelectElems);
    M.FormSelect.init(membersSelectElems);

    var elems = document.querySelectorAll('.collapsible');
    var instances = M.Collapsible.init(elems);

    var modalElem = document.querySelector('#delete-files-modal');
    deleteFileModalInstance = M.Modal.init(modalElem);

    var fcsInfoModalElem = document.querySelector('#fcs-info-modal');
    fcsInfoModalInstance = M.Modal.init(fcsInfoModalElem, {
      onOpenEnd: function (modal, trigger) {
        $("#fcs-info-modal").scrollTop(0);
      }
    });

    $(".not-collapse").on("click", function(e) { e.stopPropagation(); });

    $('#owners-${experiment?.id}').change(function() {
      const values = $(this).val();
      if (values) {
        values.map(value => $('#members-${experiment?.id} option[value=' + value + ']')
            .removeAttr('selected'))
        M.FormSelect.init(membersSelectElems);
      }
    });
    $('#members-${experiment?.id}').change(function () {
      const values = $(this).val();
      if (values) {
        values.map(value => $('#owners-${experiment?.id} option[value=' + value + ']')
            .removeAttr('selected'))

        M.FormSelect.init(ownersSelectElems);
      }
    });

    $("#confirm-file-deletion").click(function () {
      var fileIds = [];
      $('input[type="checkbox"][name="selected-files"]:checked').each(function () {
        fileIds.push($(this).val());
      });

      $.ajax({
        url: "${createLink(controller: 'expFile', action: 'deleteMultiple')}",
        data: {"fileIds": JSON.stringify(fileIds), "expId": ${experiment?.id} },
        type: "POST",
        success: function (data) {
          if(data.success) {
            M.toast({
              html: '<span>Files are successfully deleted!</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
              displayLength: Infinity
            });

            if (data.fileCount === 0) {
              window.location.href = window.location.href.replace(/\!$/, '').replace(/\#$/, '') + '#fcs-files';
              location.reload();
            }

            for(id of fileIds) {
              $("#file-box-"+id).remove();
            }

            $("#fileCount").text(data.fileCount);
          } else {
            M.toast({
              html: '<span>Deletion failed! - ' + data.msg + '</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
              displayLength: Infinity,
              classes: 'red'
            });
          }
        }
      });
    })

    $('#switch-file-selection').change(function () {
      if (this.checked)
        $('.checkbox-label').show();
      else
        $('.checkbox-label').hide();
    });

    $('#select-all-file').change(function () {
      if (this.checked)
        $('.checkbox-files').prop("checked", true);
      else
        $('.checkbox-files').prop("checked", false);
    });

    $("#confirm-gate-drawing").click(function () {
      $("#gate-drawing-error").hide();
      var id = $("#selected-fcs-file").val();
      var transformation = [];
      var hasError = false;

      // Loop through grabbing everything
      var $rows = $("tbody tr").each(function (index) {
        var $cells = $(this).find("td");
        transformation[index] = {};

        $cells.each(function (cellIndex) {
          if (cellIndex === 0) {
            transformation[index]["visibleForGating"] = $(this).find('input[type="checkbox"]').is(':checked') ? 1 : 0;
          } else if (cellIndex === 2) {
            transformation[index]["name"] = $(this).text();
          } else if (cellIndex === 3) {
            transformation[index]["longName"] = $(this).children('input[type="text"]').val();
          } else if (cellIndex === 4) {
            var selectedTransform = $(this).find('input[type=radio]:checked').val();

            //No need to add additional data for none
            if(selectedTransform !== 'none') {
              transformation[index]["defaultTransform"] = {};
              transformation[index]["defaultTransform"]["transformType"] = selectedTransform;
            } else {
              transformation[index]["defaultTransform"] = null;
            }

            if (selectedTransform === 'predefined') {
              var select = $(this).find('select[id$="transform_predefined"]');

              transformation[index]["defaultTransform"]["name"] = select.val();
            } else if (selectedTransform === 'linear') {
              var linearA = $(this).find('input[type="number"][id$="linear_a"]');
              var linearT = $(this).find('input[type="number"][id$="linear_t"]');

              var linearAValue = linearA.val();
              var linearTValue = linearT.val();

              if (linearAValue === "") {
                linearA.addClass("invalid");
                hasError = true;
              }
              if (linearTValue === "") {
                linearT.addClass("invalid");
                hasError = true;
              }

              transformation[index]["defaultTransform"]["a"] = parseFloat(linearAValue);
              transformation[index]["defaultTransform"]["t"] = parseFloat(linearTValue);
            } else if (selectedTransform === 'log') {
              var logM = $(this).find('input[type="number"][id$="log_m"]');
              var logT = $(this).find('input[type="number"][id$="log_t"]');

              var logMValue = logM.val();
              var logTValue = logT.val();

              if (logMValue === "") {
                logM.addClass("invalid");
                hasError = true;
              }
              if (logTValue === "") {
                logT.addClass("invalid");
                hasError = true;
              }

              transformation[index]["defaultTransform"]["m"] = parseFloat(logMValue);
              transformation[index]["defaultTransform"]["t"] = parseFloat(logTValue);
            } else if (selectedTransform === 'logicle') {
              var logicleA = $(this).find('input[type="number"][id$="logicle_a"]');
              var logicleT = $(this).find('input[type="number"][id$="logicle_t"]');
              var logicleM = $(this).find('input[type="number"][id$="logicle_m"]');
              var logicleW = $(this).find('input[type="number"][id$="logicle_w"]');

              var logicleAValue = logicleA.val();
              var logicleTValue = logicleT.val();
              var logicleMValue = logicleM.val();
              var logicleWValue = logicleW.val();

              if (logicleAValue === "") {
                logicleA.addClass("invalid");
                hasError = true;
              }
              if (logicleTValue === "") {
                logicleT.addClass("invalid");
                hasError = true;
              }
              if (logicleMValue === "") {
                logicleM.addClass("invalid");
                hasError = true;
              }
              if (logicleWValue === "") {
                logicleW.addClass("invalid");
                hasError = true;
              }

              transformation[index]["defaultTransform"]["a"] = parseFloat(logicleAValue);
              transformation[index]["defaultTransform"]["t"] = parseFloat(logicleTValue);
              transformation[index]["defaultTransform"]["m"] = parseFloat(logicleMValue);
              transformation[index]["defaultTransform"]["w"] = parseFloat(logicleWValue);
            }
          }
        });
      });

      if (hasError) {
        $("#gate-drawing-error").show();
        return false;
      }

      $.ajax({
        url: "${createLink(controller: 'expFile', action: 'processGateDrawing')}/" + id,
        dataType: 'json',
        type: "POST",
        data: {"transformation": JSON.stringify(transformation), "updateData": document.getElementById('updateData').checked},
        success: function (data) {
          if (data.success) {
            window.open(data.url, '_blank');
          } else {
            M.toast({
              html: '<span>' + data.msg + '</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
              displayLength: Infinity,
              classes: 'red'
            });
          }
        },
        error: function (request, status, error) {
          console.log('E: ' + error);
        },
      });
    });
  });

  function openDeleteFileModal() {
    var totalChecked = $('input[type="checkbox"][name="selected-files"]:checked').length;
    if(totalChecked == 0) {
      M.toast({
        html: '<span>No file selected!</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
        displayLength: Infinity,
        classes: 'red'
      });
    } else {
      deleteFileModalInstance.open();
    }

  }

  function renderFCSFileInfo(id) {
    $("#updateData").prop("checked", true); //check update data option as default

    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'renderFCSFileInfo')}/" + id,
      dataType: 'json',
      type: "POST",
      success: function (data) {
        if (data.success) {
          $("#fcs-info-modal .modal-content").html(data.modelContent);

          var elems = document.querySelectorAll('.fcs_param_transform');
          var instances = M.FormSelect.init(elems);
          fcsInfoModalInstance.open();
        } else {
          M.toast({
            html: '<span>' + data.msg + '</span><button class="btn-flat btn-small toast-action" onclick="$(this).parent().remove()"><i class="material-icons">close</i></button>',
            displayLength: Infinity,
            classes: 'red'
          });
        }
      }
    });
  }
</script>
