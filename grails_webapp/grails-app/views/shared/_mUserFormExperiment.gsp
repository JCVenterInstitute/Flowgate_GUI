<%@ page import="flowgate.User; flowgate.ExperimentUser" %>
<g:form class="form-horizontal" name="manageUsers" controller="experiment" action="manageUsers" id="${experiment?.id}">
  <div class="modal-header">
    <div class="btn-group pull-left">
      <button class="btn btn-danger" data-dismiss="modal">
        Cancel
      </button>
    </div>

    <div class="btn-group pull-right">
      <button type="submit" class="btn btn-success">
        Save
      </button>
    </div>

    <h3 class="modal-title">Manage Users (${experiment?.title})</h3>
  </div>

  <div class="modal-body">
    <div class="form-group" style="padding-bottom: 10px">
      <label class="col-sm-2 control-label">
        owner(s)
      </label>

      <div class="col-sm-10">
        <script type="text/javascript">
          $(document).ready(function () {
            $(".owners-${experiment?.id}").select2();
          });
        </script>
        <g:select style="width: 100%"
                  class="select2-container owners-${experiment?.id} form-control"
                  name="owners"
                  value="${ExperimentUser?.findAllByExperimentAndExpRole(experiment, 'owner')*.user*.id}"
                  from="${User.list()}"
                  optionKey="id"
                  optionValue="username"
                  multiple=""/>
      </div>
    </div>

    <div class="form-group">
      <label class="col-sm-2 control-label">
        member(s)
      </label>

      <div class="col-sm-10">
        <script type="text/javascript">
          $(document).ready(function () {
            $(".members-${experiment?.id}").select2();
          });
        </script>
        <g:select style="width: 100%"
                  class="select2 multiSelect members-${experiment?.id} form-control"
                  id="members-${experiment?.id}"
                  name="members"
                  value="${ExperimentUser?.findAllByExperimentAndExpRole(experiment, 'member')*.user*.id}"
                  from="${User.list()}"
                  optionKey="id"
                  optionValue="username"
                  multiple=""/>
      </div>
    </div>
  </div>
  <div class="modal-footer"></div>
</g:form>
