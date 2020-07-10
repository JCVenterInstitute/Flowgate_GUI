/*
   * Adds a new item.
   */
function addItem(id, elem, min, max, onComplete, limitMessage, removeBtnLabel, mdVals) {
  // checks if we have reached maximum number of elements
  if (!max || $('[id^=' + id + ']').length < max) {
    // increments the item counter
    var $countElem = $('#count_' + id);
    var num = parseInt($countElem.html()) + 1;
    $countElem.html(num);
    var myArr = [mdVals.replace('[','').replace(']','').split(', ')];
    var myArrStr = myArr[0][num-1] ? myArr[0][num-1]+ '" ' : '' + '" ';
    // console.log(elem);
    // console.log(myArrStr);
    elem = elem.replace('id="mdValue"','id="mdValue" value="'+myArrStr);
    // creates new item and adds the index number to it
    var $newElem = $('<div ></div>').html(elem).attr({'class': 'row', 'id' : id + num});
    // creates the "Remove" button
    let $removeButtonDiv = $('<div/>').attr({'class': 'input-field col s2'}).appendTo($newElem);
    var $removeButton = $('<input type="button"/>').appendTo($removeButtonDiv);
    $removeButton.attr({
      id: 'remove_' + id,
      class: 'waves-effect waves-light btn-flat',
      value: removeBtnLabel ? removeBtnLabel : 'Remove',
      disabled: 'disabled'
    });
    // binds handler to the 'click' JS event of the "Remove" button
    $removeButton.click(function() {
      removeItem(id, num, min);
    });
    // changes IDs of all elements inside new item
    indexItem($newElem, num);
    // appends new item to the parent element
    $('#parent_' + id).append($newElem);
    // enables "Remove" buttons if there are more than minimum number of elements on the page
    if ($('[id^=' + id + ']').length > min) {
      $('[id^=remove_' + id + ']').removeAttr('disabled');
    }
    // executes the 'onComplete' JS function if it exists
    if (window[onComplete] instanceof Function) {
      window[onComplete](num);
    }
  } else {
    // displays a message if the maximum limit is reached
    alert(limitMessage ? limitMessage : 'You cannot add more elements.');
  }
}

/*
 * Removes an item.
 */
function removeItem(id, num, min) {
  $('#' + id + num).remove();
  if ($('[id^=' + id + ']').length <= min) {
    $('[id^=remove_' + id + ']').attr('disabled', 'disabled');
  }
}

/*
 * Changes ID of every item's child by adding the index number to it.
 */
function indexItem($elem, num) {
  $elem.children().each(function() {
    var nodeId = $(this).attr('id');
    if (nodeId) {
      $(this).attr('id', nodeId + num);
    }
    indexItem($(this), num);
  });
}
