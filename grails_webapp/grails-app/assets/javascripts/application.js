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

var offset = 250;
var duration = 300;

$(window).scroll(function () {
  if ($(this).scrollTop() > offset) {
    $('#scrollToTop').fadeIn(duration);
  } else {
    $('#scrollToTop').fadeOut(duration);
  }

  if ((($(document).height() - $(this).height()) - $(this).scrollTop()) > offset) {
    $('#scrollToBottom').fadeIn(duration);
  } else {
    $('#scrollToBottom').fadeOut(duration);
  }
});

$('#scrollToTop').click(function (event) {
  $('html, body').animate({scrollTop: 0}, duration);
  return false;
});

$('#scrollToBottom').click(function (event) {
  $('html, body').animate({scrollTop: $(document).height()}, duration);
  return false;
});

$("#breadcrumbs_unfold").click(function () {
  $('.breadcrumb.tooltipped').each(function (i, obj) {
    const tooltip = $(obj).attr('data-tooltip');
    const text = $(obj).text();
    $(obj).attr('data-tooltip', text);
    $(obj).text(tooltip);
  });

  if ($(this).text() === 'unfold_more') {
    $(this).text('unfold_less');
  } else {
    $(this).text('unfold_more');
  }
});
