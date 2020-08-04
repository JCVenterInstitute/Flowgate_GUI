// This is a manifest file that'll be compiled into application.js.
//
// Any JavaScript file within this directory can be referenced here using a relative path.
//
// You're free to add application-wide JavaScript to this file, but it's generally better
// to create separate JavaScript files as needed.
//
//= require select2/select2.min
//= require bootstrap-all
// = require bootcards/bootcards.min //just needed for offcanvas, automatic portrait mode
// = require_tree .
//= require_self

if (typeof jQuery !== 'undefined') {
  (function ($) {
    $(document).ajaxStart(function () {
      $('#spinner, #screen-locker').fadeIn();
    }).ajaxStop(function () {
      $('#spinner, #screen-locker').fadeOut();
    });
  })(jQuery);

  $(document).ready(function () {
    $(".breadcrumb a").each(function () {
      $(this).text(shorten($(this).text(), 50));
    });
  });
}

function shorten(text, count) {
  return text.slice(0, count) + (text.length > count ? "..." : "");
}

function updatePaginationToMaterial(pagination) {
  var hasNextLink = false, hasPrevLink = false;

  pagination.children().each(function(index,html) {
    var $this = $(this);
    if($this.hasClass('currentStep')){
      $this.wrap("<li class='active'></li>");
      $this.wrapInner("<a href='#!'></a>");
    } else{
      $this.wrap("<li class='waves-effect'></li>");

      if($this.hasClass('nextLink')){
        $this.wrapInner("<i class='material-icons'></i>");
        hasNextLink = true;
      }

      if($this.hasClass('prevLink')){
        $this.wrapInner("<i class='material-icons'></i>");
        hasPrevLink = true;
      }
    }
  });

  if(!hasNextLink) {
    $('#pagination').append('<li class="disabled"><a href="#!"><i class="material-icons">chevron_right</i></a></li>');
  }

  if(!hasPrevLink) {
    $('#pagination').prepend('<li class="disabled"><a href="#!"><i class="material-icons">chevron_left</i></a></li>');
  }
}
