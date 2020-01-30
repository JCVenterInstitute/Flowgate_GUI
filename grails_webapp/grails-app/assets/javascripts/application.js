// This is a manifest file that'll be compiled into application.js.
//
// Any JavaScript file within this directory can be referenced here using a relative path.
//
// You're free to add application-wide JavaScript to this file, but it's generally better
// to create separate JavaScript files as needed.
//
//= require materialize/js/bin/materialize
// = require_tree .
//= require_self

if (typeof jQuery !== 'undefined') {
  (function ($) {
    $(document).ajaxStart(function () {
      $('.preload-background').css('display', 'flex');
    }).ajaxStop(function () {
      $('.preload-background').fadeOut();
    });
  })(jQuery);

  $(document).ready(function () {
    $(".dropdown-trigger").dropdown({ hover: false });
    $("a.breadcrumb").each(function () {
      $(this).text(shorten($(this).text(), 50));
    });
  });
}

document.addEventListener('DOMContentLoaded', function () {
  var tooltipElems = document.querySelectorAll('.tooltipped');
  M.Tooltip.init(tooltipElems);
});

function shorten(text, count) {
  return text.slice(0, count) + (text.length > count ? "..." : "");
}
