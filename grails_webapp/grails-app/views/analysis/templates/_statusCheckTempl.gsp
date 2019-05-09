<script>
  //var intrvalTmr = setInterval(checkTimer, 3000);

  function checkTimer() {
    if ($('.modal.in').length <= 0) {
      $.ajax({
        url: "${createLink(controller: 'analysis', action: 'checkStatus')}",
        dataType: "json",
        type: "get",
        global: false,
        data: {"eId": ${params?.eId}, "jobs": JSON.stringify(${jobList})},
        success: function (data) {
          console.log("status update");
        },
        error: function (request, status, error) {
          console.log("E: in checkStatus! something went wrong!!!")
        },
        complete: function () {
          console.log('ajax completed');
        }
      });

      $.ajax({
        url: "${createLink(controller: 'analysis', action: 'checkDbStatus')}",
        dataType: "json",
        type: "get",
        global: false,
        data: {"eId": ${params?.eId}, "jobs": JSON.stringify(${jobList})},
        async: true,
        success: function (data) {
          if (data.updChkStatus == "clear") {
            console.log('do reset');
//                    window.clearInterval(intrvalTmr);
            clearInterval(intrvalTmr);
          }
          var pageNumber = analysisTable.page();
          $("#analysisListTabl").html(data.tablTempl);

          analysisTable = $("#analysis-table").DataTable({
            "columnDefs": [
              {"type": "date-euro", targets: 2}
            ],
            "order": [[2, "desc"]]
          });

          analysisTable.page(pageNumber).draw('page');
          setCurrentTime();
        },
        error: function (request, status, error) {
          console.log("E: in checkDbStatus! something went wrong!!!")
        },
        complete: function () {
          console.log('ajax completed');
        }
      });
    }
  }

</script>
