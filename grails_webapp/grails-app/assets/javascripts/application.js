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
