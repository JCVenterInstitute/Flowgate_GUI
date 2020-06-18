<script>
  var jobs = ${jobList};
  if (jobs !== undefined && jobs.length != 0) {
    window.setInterval(function () {
      if ($('.modal.in').length <= 0) {
        for (i in jobs) {
          var server = $("#server-"+jobs[i]).val();
          $.ajax({
            url: server + "/gp/rest/v1/jobs/" + jobs[i] + "/status.json",
            beforeSend: function (xhr) {
              xhr.setRequestHeader("Authorization", "Basic Zmxvd2dhdGU6Zmxvd0dhdGU=");
            },
            success: function (resp) {
              console.log(resp.isFinished);
            }
          });
        }
      }

      if (jobs === undefined || jobs.length == 0) clearInterval();
    }, 5000);
  }

  function updateStatus() {
    if ($('.modal.in').length <= 0) {
      $.ajax({
        url: "${createLink(controller: 'analysis', action: 'updateStatus')}",
        dataType: "json",
        type: "get",
        global: false,
        data: {"eId": ${params?.eId}, "jobs": JSON.stringify(${jobList})},
        success: function (data) {
          var disableUpdate = true;
          for (var key in data.status) {
            var result = data.status[key]
            if (result != 2) {
              var $html = $.parseHTML(result);
              var row = analysisTable.row('#job' + key);
              var rData = row.data();
              var tds = $($html[0]).find("td")
              rData[0] = tds.eq(0).html();
              rData[1] = tds.eq(1).html();
              rData[2] = tds.eq(2).html();
              rData[3] = tds.eq(3).html();
              rData[4] = tds.eq(4).html();
              row.data(rData).invalidate();
            } else {
              disableUpdate = false;
            }
          }
          if (disableUpdate) $("#updateStatusBtn").attr("disabled", true);
        },
        error: function (request, status, error) {
          console.log("E: in updateStatus!")
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }
  }

</script>
