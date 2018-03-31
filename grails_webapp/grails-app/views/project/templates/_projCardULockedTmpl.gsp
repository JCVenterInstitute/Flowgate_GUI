%{--<a class="noLinkBlack" href="${createLink(controller: 'project', action: 'index', params:[pId: project?.id])}">--}%
<a class="noLinkBlack" href="/flowgate/project/index?pId=${project?.id}" >
    <div class="panel panel-default">
        <div class="bootcards-summary-item ${session?.searchLst?.find {it == project?.id} != null ? 'findSel' : ''}">
            <div class="panel-body">
                %{--<i class="fa fa-3x fa-file-text-o" style="padding-bottom: 10px;"></i>--}%
                <i class="fa fa-3x fa-${session?.projectOpenId?.toLong() == project?.id ? 'folder-open' : 'folder'}-o" style="padding-bottom: 10px;"></i>
                <h4>${project?.title}
                    <span class="label label-success"><i class="fa fa-unlock"></i></span>
                </h4>
            </div>
        </div>
        <div class="panel-footer">
            <div class="row">
                <div class="col-xs-1 pull-left">
                    <i class="fa fa-user"></i>
                </div>
                <div class="col-xs-8 text-center">
                    <i class="fa fa-info"></i>
                </div>
                <div class="col-xs-1 pull-right">
                    <i class="fa fa-gear"></i>
                </div>
            </div>
        </div>
    </div>
</a>