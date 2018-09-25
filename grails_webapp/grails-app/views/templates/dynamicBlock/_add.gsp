<div class="row">
  <div class="col-sm-12 form-inline">
    <div id="count_${id}" style="display: none;">0</div>
    <div id="parent_${id}"></div>
  </div>
</div>
<g:if test="${!addBtnId}">
  <input id="add_${id}" type="button" value="Add possible values for this attribute"/>
</g:if>
<script>
  function initializeTag(addButton, id, elem, min, max, onComplete, limitReachedMsg, removeBtnLabel, mdVals, ms) {
    // binds event handler to the "click" JS event of the "Add" button
    addButton.click(function() {
      // alert('add clicked!');
      addItem(id, elem, min, max, onComplete, limitReachedMsg, removeBtnLabel, mdVals);
    });

    // adds the initial number of items
    // for (var i = 0; i < min; i++) {
    for (var i = 0; i < ms; i++) {
      addButton.click();
    }
  }

  $(function () {
    // gets the "Add" button
    var addButton = $("#add_${addBtnId ? addBtnId : id}");

      // imports the dynamicBlock.js file if it has not been imported yet
    if (!window["addItem"]) {
      $.getScript("${resource(dir: "javascripts", file: "dynamicBlock.js")}", function() {
        initializeTag(addButton, "${id}", "${elem}", ${min}, ${max}, "${onComplete}",
          "${limitReachedMsg}", "${removeBtnLabel}", "${mdVals}", ${mdVals.size()} );
      });
    } else {
      initializeTag(addButton, "${id}", "${elem}", ${min}, ${max}, "${onComplete}",
        "${limitReachedMsg}", "${removeBtnLabel}", "${mdVals}", ${mdVals.size()} );
    }
  }
  );
</script>
