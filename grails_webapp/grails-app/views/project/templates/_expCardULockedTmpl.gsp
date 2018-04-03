<%@ page import="flowgate.Experiment; flowgate.ExperimentUser" %>
<div class="panel panel-default">
  <div class="bootcards-summary-item">
    <a class="noLinkBlack" href="/flowgate/experiment/index?eId=${experiment?.id}">
      <div class="panel-body">
        <i class="fa fa-3x fa-${session?.experimentOpenId?.toLong() == experiment?.id ? 'folder-open' : 'folder'}-o"
           style="padding-bottom: 10px;"></i>
        <h4>${experiment?.title}
          <span class="label label-success"><i class="fa fa-unlock"></i></span>
        </h4>
      </div>
    </a>
  </div>

  <div class="panel-footer">
    <div class="row">
      <div class="col-xs-1 pull-left">
        <div class="btn-toolbar btn btn-default" data-toggle="modal"
             data-target="#manageProjExpUsersModal-${experiment?.id}">
          %{--<button type="button" class="" data-toggle="modal" data-target="#manageUsersModal-${expFile?.id}"><i class="fa fa-users"></i></button>--}%
          <i class="fa fa-user">&nbsp;<span
               class="label label-pill label-success">${ExperimentUser.findAllByExperiment(experiment as Experiment)?.size()}</span>
          </i>
        </div>
      </div>

      <div class="col-xs-7 text-center">
        <i class="fa fa-info"></i>
      </div>

      <div class="col-xs-1 pull-right">
        <i class="fa fa-gear"></i>
      </div>
    </div>
  </div>
</div>