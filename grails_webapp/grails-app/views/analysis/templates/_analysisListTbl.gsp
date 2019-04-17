<table id="analysis-table" class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <th>Task Name</th>
    <th>Reports</th>
    <th>Task Status</th>
    <th>Submitted on</th>
    <th>Creator</th>
  </tr>
  </thead>
  <tbody>
  <g:each in="${analysisList}" var="bean" status="i">
    <g:render template="templates/analysisListRow" model="[bean: bean]"/>
  </g:each>
  </tbody>
</table>
