/*
 * jQuery File Upload Plugin JS Example
 * https://github.com/blueimp/jQuery-File-Upload
 *
 * Copyright 2010, Sebastian Tschan
 * https://blueimp.net
 *
 * Licensed under the MIT license:
 * https://opensource.org/licenses/MIT
 */

/* global $, window */

$(function () {
  $('#uploadFile').fileupload({
    // maxChunkSize: 1000000,
    // multipart: false,
    global: false,
    add: function(e, data) {
      //file extension validation for .csv and .zip
      var acceptFileTypes = /(\.)(fcs|zip)$/i;

      var acceptable = data.files[0]['name'].length && acceptFileTypes.test(data.files[0]['name']);
      var $fileRow = $('<div class="row file"/>').append(
          $('<div class="col-md-12"/>').append(
              $('<label>').html(data.files[0]['name']+ " (" + formatFileSize(data.files[0]['size']) + ")"))
          );

      if(!acceptable) {
        var $error = $('<span class="label label-danger" style="margin-left: 10px;"/>').text("Only .fcs and .zip files are supported");
        $fileRow.find(".col-md-12").append($error);
        data.files.length = 0;
      } else if(data.files[0]['size'] > 1050000000) {
        var $error = $('<span class="label label-danger" style="margin-left: 10px;"/>').text("File cannot be larger then 1GB");
        $fileRow.find(".col-md-12").append($error);
        data.files.length = 0;
      } else {
        var $progressBar = $('<div class="col-md-12">').append($('<div/>', {class: 'progress', id: 'status-progress', style: 'display:none;'})
            .append($('<div/>', {class: 'progress-bar progress-bar-striped progress-bar-animated', role: 'progressbar', 'aria-valuemin': '0', 'aria-valuemax' : '100'})));

        var $cancelBtn = $('<button type="button" class="btn btn-xs btn-warning cancel" style="margin-left:10px;" />').text("Cancel").click(function() {
          data.abort();
          data.files.length = 0;
          $(this).closest('.file').remove();
        });
        $fileRow.find(".col-md-12").append($cancelBtn);
        $fileRow.append($progressBar);
      }
      data.context = $fileRow.appendTo($('#files'));

      $('#uploadFilesBtn').on('click', function(e) {
        e.preventDefault();
        if (data.files.length > 0) {
          data.submit();
        }
      });
    },
    dataType: 'json',
    sequentialUploads: true,
    done: function (e, data) {
      $('#files').find('.file').each(function(i,v) {
        var $node = $(v);

        if($node.text().indexOf(data.files[0].name) >= 0) {
          if($node.find('span.label').length < 1) {
            if(data.result.success == true)
              $node.find(".col-md-12:first-child").append($('<span class="label label-info" style="margin-left: 10px;"/>').text("Uploaded Successfully"));
            else
              $node.find(".col-md-12:first-child").append($('<span class="label label-danger" style="margin-left: 10px;"/>').text("Failed to upload! - " + data.result.msg));

            $node.find('button').hide();  // remove cancel button
          }
        }
      });
      data.files.length = 0;
    },
    fail: function (e, data) {
      $('#files').find('.file').each(function(i,v) {
        var $node = $(v);

        if($node.text().indexOf(data.files[0].name) >= 0) {
          if($node.find('span.label').length < 1) {
            $node.find(".col-md-12:first-child").append($('<span class="label label-danger" style="margin-left: 10px;"/>')
                .text("Failed to upload! " + ((data.result && data.result.msg) ? " - " + data.result.msg : "")));
            $node.find('button').hide();
          }
        }
      });
    },
    dropZone: $('#dropzone')
  });
});

function formatFileSize (bytes) {
  if(bytes === 0) { return "0.00 B"; }
  var e = Math.floor(Math.log(bytes) / Math.log(1024));
  return (bytes/Math.pow(1024, e)).toFixed(2)+' '+' KMGTP'.charAt(e)+'B';
}