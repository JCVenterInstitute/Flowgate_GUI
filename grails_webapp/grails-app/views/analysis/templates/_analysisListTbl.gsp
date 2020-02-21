<table id="analysis-table" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <th>Task Name</th>
    <th>Reports</th>
    <th>Task Status</th>
    <th>Submitted on</th>
    <th>Creator</th>
%{--    <th></th>--}%
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <tr id="${bean?.jobNumber}"
      <g:render template="templates/analysisListTablRow" model="[bean: bean]"  />
    </tr>
  </g:each>
  </tbody>
</table>
