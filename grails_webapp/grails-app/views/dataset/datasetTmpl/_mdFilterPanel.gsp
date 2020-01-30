<g:if test="${experiment?.expMetadatas?.size() > 0}">
  <style>
  #metaBox {
    border: black 1px solid;
    overflow-y: scroll;
    overflow-x: hidden;
    max-height: 700px;
  }
  </style>
  <a class="s_h_filter" style="color:#333333; cursor: pointer;">
    <p><strong>Filter</strong><i class="material-icons">arrow_drop_up</i></p>
  </a>

  <div id="metaData">
    <div id="metaBox">
      <g:each in="${experiment?.expMetadatas?.findAll { it.dispOnFilter }?.sort { it.mdCategory }?.mdCategory?.unique()}" var="catgy">
        <g:each in="${experiment?.expMetadatas?.findAll { it.mdCategory == catgy && it.dispOnFilter }?.sort { it.dispOrder }}" var="catGy">
          <g:if test="${experiment?.expMetadatas?.find { it.mdKey == catGy.mdKey && it.dispOnFilter }?.mdVals?.size() > 0}">
            <div class="col s4">
              <p><strong>${catGy.mdKey}</strong></p>
              <g:each in="${experiment?.expMetadatas?.find { it.mdKey == catGy.mdKey && it.dispOnFilter }?.mdVals?.sort { it.dispOrder }}" var="mVals">
                <p><label><g:checkBox id="${mVals.mdValue}" class="fcs_filter filled-in" name="mVal_${mVals.id}_chb" onclick="setFilter();"
                                      checked="${mVals.mdValue in params.filters}"/>
                  <span>${mVals.mdValue}</span>
                </label></p>
              </g:each>
            </div>
          </g:if>
        </g:each>
      </g:each>
    </div>
  </div>
</g:if>
