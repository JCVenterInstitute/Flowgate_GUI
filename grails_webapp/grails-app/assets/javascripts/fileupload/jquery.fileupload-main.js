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
    add: function (e, data) {
      //file extension validation for .csv and .zip
      var acceptFileTypes = /(\.)(fcs|zip)$/i;

      var acceptable = data.files[0]['name'].length && acceptFileTypes.test(data.files[0]['name']);
      var $fileRow = $('<li/>', {class: 'collection-item avatar file'})
          .append($('<i/>', {class: 'material-icons circle'}).text('file_upload'))
          .append($('<span/>', {class: 'title'}).html(data.files[0]['name'] + " (" + formatFileSize(data.files[0]['size']) + ")")
          );

      if (!acceptable) {
        var $error = $('<p/>', {class: 'red-text'}).text("Only .fcs and .zip files are supported");
        $fileRow.append($error);
        data.files.length = 0;
      } else if (data.files[0]['size'] > 1050000000) {
        var $error = $('<p/>', {class: 'red-text'}).text("File cannot be larger then 1GB");
        $fileRow.append($error);
        data.files.length = 0;
      } else {
        var $progressBar = $('<div/>', {class: 'progress', id: 'status-progress'})
            .append($('<div/>', {class: 'determinate', style: 'width: 0%'}));

        var $cancelBtn = $('<a/>', {href: '#!', class: 'secondary-content btn-flat', style: 'padding: 0; line-height: 1.5rem'})
            .append($('<i/>', {class: 'material-icons'}).text('clear'))
            .click(function () {
              data.abort();
              data.files.length = 0;
              $(this).closest('.file').remove();
            });

        $fileRow.append($('<p/>').append('<span/>').append('<br/>').append($progressBar))
            .append($cancelBtn);
      }
      data.context = $fileRow.appendTo($('#files'));

      $('#uploadFilesBtn').on('click', function (e) {
        e.preventDefault();
        if (data.files.length > 0) {
          data.submit();
        }
      });
    },
    dataType: 'json',
    sequentialUploads: true,
    done: function (e, data) {
      $('#files').find('.file').each(function (i, v) {
        var $node = $(v);

        if ($node.text().indexOf(data.files[0].name) >= 0) {
          if ($node.find('span.label').length < 1) {
            if (data.result.success == true)
              $node.find("p > span").addClass('green-text').text("Uploaded Successfully");
            else
              $node.find("p > span").addClass('red-text').text("Failed to upload! - " + data.result.msg);

            $node.find('.secondary-content').hide();  // remove cancel button
          }
        }
      });
      data.files.length = 0;
    },
    fail: function (e, data) {
      $('#files').find('.file').each(function (i, v) {
        var $node = $(v);

        if ($node.text().indexOf(data.files[0].name) >= 0) {
          if ($node.find('span.label').length < 1) {
            $node.find("p > span").addClass('red-text').text("Failed to upload! - " + ((data.result && data.result.msg) ? " - " + data.result.msg : ""));
            $node.find('.secondary-content').hide();
          }
        }
      });
    },
    dropZone: $('#dropzone')
  });
});

function formatFileSize(bytes) {
  if (bytes === 0) {
    return "0.00 B";
  }
  var e = Math.floor(Math.log(bytes) / Math.log(1024));
  return (bytes / Math.pow(1024, e)).toFixed(2) + ' ' + ' KMGTP'.charAt(e) + 'B';
}