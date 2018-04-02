module
<table class="table table-bordered table-responsive table-striped table-hover dataTable" width="100%">
  <thead>
  <tr>
    <g:each in="${domainProperties}" var="p" status="i">
      <g:set var="propTitle">
        ${domainClass.propertyName}.${p.name}.label
      </g:set>
      <g:sortableColumn property="${p.name}" title="${message(code: propTitle, default: p.naturalName)}"/>
    </g:each>
  </tr>
  </thead>
  <tbody>
  <g:each in="${collection}" var="bean" status="i">
    <tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
      <g:each in="${domainProperties}" var="p" status="j">
        <g:if test="${j == 0}">
          <td>
            %{--<sec:ifAnyGranted roles="ROLE_Administrator">--}%
            %{--<g:link method="GET" resource="${bean}">--}%
            <g:link action="edit" resource="${bean}">
              <f:display bean="${bean}" property="${p.name}" displayStyle="${displayStyle ?: 'table'}"/>
            </g:link>
            %{--</sec:ifAnyGranted>--}%
            %{--<sec:ifAnyGranted roles="ROLE_Administrator">--}%
            %{--  Show User method  --}%
            %{--<g:link method="GET" resource="${bean}">
                <f:display bean="${bean}"
                           property="${p.name}"
                           displayStyle="${displayStyle?:'table'}" />
            </g:link>--}%
            %{--</sec:ifAnyGranted>--}%
            %{--<sec:ifNotGranted roles="ROLE_Administrator">--}%
            %{--<f:display bean="${bean}"--}%
            %{--property="${p.name}"--}%
            %{--displayStyle="${displayStyle?:'table'}" />--}%
            %{--</sec:ifNotGranted>--}%
          </td>
        </g:if>
        <g:else>
          <td>
            <f:display bean="${bean}" property="${p.name}" displayStyle="${displayStyle ?: 'table'}"/>
          </td>
        </g:else>
      </g:each>
    </tr>
  </g:each>
  </tbody>
</table>