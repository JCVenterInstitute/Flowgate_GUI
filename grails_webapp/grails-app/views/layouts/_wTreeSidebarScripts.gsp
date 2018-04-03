<script type="text/javascript">

  function setFilter() {
    var filterString = document.getElementById("filterInput").value;
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axSearchTree')}",
      dataType: "json",
      type: "get",
      data: {filterString: filterString},
      success: function (data) {
        $("#projTree").html(data.contentTree);
      },
      error: function (request, status, error) {
        alert(error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function clearFilter() {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axClearSearchTree')}",
      dataType: "json",
      type: "get",
      success: function (data) {
        $("#projTree").html(data.contentTree);
      },
      error: function (request, status, error) {
        alert(error);
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function projectClick(projId) {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axProjectClick')}",
      dataType: 'json',
      type: "get",
      data: {projId: projId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#topBtnBar").html(data.contentBtnBar);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function projectCloneClick(projId) {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axCloneProjectClick')}",
      dataType: 'json',
      type: "get",
      data: {projId: projId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#topBtnBar").html(data.contentBtnBar);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('clone project completed!');
      }
    });
  }

  function experimentClick(projId, expId) {
    $.ajax({
      url: "${createLink(controller: 'experiment', action: 'axExperimentClick')}",
      dataType: 'json',
      type: "get",
      data: {projId: projId, expId: expId},
      success: function (data) {
        console.log('had success!!');
        $("#projTree").html(data.contentTree);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function experimentUploadTemplateClick() {
    alert('not yet implemented!');
  }


  function experimentCloneClick(experimentId) {
    $.ajax({
      url: "${createLink(controller: 'experiment', action: 'axCloneExperimentClick')}?eId=" + experimentId,
      dataType: 'json',
      type: "put",
      data: {eId: experimentId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#topBtnBar").html(data.contentBtnBar);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('clone experiment completed!');
      }
    });
  }

  function toggleExpEditMode(experimentId) {
    $.ajax({
      url: "${createLink(controller: 'experiment', action: 'toggleExperimentEditModeClick')}",
      dataType: 'json',
      type: "get",
      data: {eId: experimentId},
      success: function (data) {
        console.log('had success!!');
        $("#topBtnBar").html(data.contentBtnBar);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function expFileCollapseAllClick(experimentId) {
    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'collapseAll')}",
      dataType: 'json',
      type: "get",
      data: {eId: experimentId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function expFileExpandAllClick(experimentId) {
    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'expandAll')}",
      dataType: 'json',
      type: "get",
      data: {eId: experimentId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function toggleExpFileOpen(experimentId, expFileId) {
    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'axExpFileToggleClick')}",
      dataType: 'json',
      type: "get",
      data: {eId: experimentId, expFileId: expFileId},
      success: function (data) {
        $("#projTree").html(data.contentTree);
        $("#expFile-" + expFileId).replaceWith($("#expFile-" + expFileId).html(data.contentPage));
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function btnAddExpFileClick(experimentId) {
    $.ajax({
      %{--url: "${createLink(controller: 'expFile', action: 'axExpFileCreate')}",--}%
      url: "${createLink(controller: 'expFile', action: 'expFileCreate')}",
      dataType: 'json',
      type: "get",
      data: {eId: experimentId},
      success: function (data) {
        console.log('had success!!');
        $("#projTree").html(data.contentTree);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });

  }

  //    TODO remove this comment once everything works fine again
  //    function fcsFileContainerDelete(fcsContainerId) {
  function expFileDelete(expFileId) {
    $.ajax({
      url: "${createLink(controller: 'expFile', action: 'axDeleteExpFile')}?expFileId=" + expFileId,
      dataType: 'json',
      type: "delete",
      success: function (data) {
        console.log('had success!!');
        $("#projTree").html(data.contentTree);
        $("#pageContent").html(data.contentPage);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }

    });
  }

  function experimentTitleInputBlur(experimentId, value) {
    $.ajax
    ({
      data: {eId: experimentId, titleValue: value},
      type: "post",
      url: "${createLink(controller: 'experiment', action: 'axExperimentTitleSave')}",
      success: function (data) {
        $("#projTree").html(data.contentTree);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  function experimentDescriptionInputBlur(experimentId, value) {
    $.ajax
    ({
      data: {eId: experimentId, descriptionValue: value},
      type: "post",
      url: "${createLink(controller: 'experiment', action: 'axExperimentDescriptionSave')}"
    });
  }

  function toggleExpView() {
    $.ajax({
      url: "${createLink(controller: 'project', action: 'axToggleExpView')}",
      dataType: 'json',
      type: "get",
      success: function (data) {
        $("#expContent").html(data.expContent);
      },
      error: function (request, status, error) {
        alert(error)
      },
      complete: function () {
        console.log('ajax completed');
      }
    });
  }

  $('[data-toggle="tooltip"]').tooltip();
  %{--</asset:script>--}%
</script>