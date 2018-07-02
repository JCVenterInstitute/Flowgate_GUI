<%@ page import="flowgate.ExpFile; flowgate.Experiment" %>
<h3>
  <div style="cursor: pointer;" onclick="window.location.href = '${createLink(controller: 'project', action: 'list')}'">${'Projects'}</div>
</h3>

%{--<div class="input-group" style="padding-right: 10px;">
  <span class="input-group-addon">
    <i class="fa fa-search"></i>
  </span>
  <input id="filterInput" class="form-control" type="text" onchange="setFilter()" placeholder="Search Projects..." value="${session?.filterString}"/>
  <g:if test="${session?.filterString != ''}">
    <span class="input-group-addon" onclick="clearFilter()"><i class="fa fa-close"></i></span>
  </g:if>
</div>
<br/>--}%
<ul>
  <g:each var="project" in="${projectList}" status="p">
    <li class="${session?.searchLst?.find { it == project?.id } != null ? 'findSel' : ''} folder" style="cursor: pointer" onclick="projectClick(${project?.id})"><i
        class="fa fa-${session?.projectOpenId?.toLong() == project?.id ? 'folder-open' : 'folder'}-o"></i>&nbsp;&nbsp;${project?.title.take(20)+'...'}</li>
    <g:if test="${project?.id == session?.projectOpenId?.toLong()}">
      <ul>
        <g:each var="experiment" in="${experimentList}">
          <li>
            <span id="experimentFolder" style="cursor: pointer" onclick="window.location.href = '${createLink(controller:'experiment', action: 'index', params: [pId: project?.id, eId: experiment?.id])}'"><i
                class="fa fa-${session?.experimentOpenId?.toLong() == experiment?.id ? 'folder-open' : 'folder'}-o"></i>
              &nbsp;&nbsp;${experiment?.title.take(20) + '...'}
              %{--&nbsp;&nbsp;{{ strTruncate(experiment.title, 30, '...') }}--}%
            </span>
            %{--<g:if test="${session?.experimentOpenId == experiment?.id}">
              <g:if test="${experiment?.expFiles?.size() > 0}">
              --}%%{-- TODO put in controller --}%%{--
                <g:each var="expFile" in="${ExpFile.findAllByExperimentAndIsActive(Experiment.findByIdAndIsActive(experiment?.id?.toLong(), true), true).sort { it.id }}">
                --}%%{--<ul (click)="boxToggle(elbox)">--}%%{--
                  <ul>
                    <li>
                      <i class="fa fa-${session?.expFileOpenIds?.contains(expFile?.id) ? 'folder-open' : 'folder'}-o"></i>&nbsp;&nbsp;${expFile?.title}
                    </li>
                  </ul>
                </g:each>
              </g:if>
            </g:if>--}%
          </li>
        </g:each>
      </ul>
    </g:if>
  </g:each>
</ul>