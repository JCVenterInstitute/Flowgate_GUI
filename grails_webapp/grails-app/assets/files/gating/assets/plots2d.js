  // var chart1 = dc.compositeChart("#pops1");

  function saveBtnClicked(){
    saveBtn.innerHTML = "saving...";
    console.log('saveBtn clicked');
    $.ajax({
      url: "/moduleInterface/writeConfigTextfile",
      data:{
        fileName:'',
        fdata:''
      },
      success: function (data) {
        // if (data.msg == "clear") {
        console.log('got ',data.msg);
//                    window.clearInterval(intrvalTmr);
//             clearInterval(intrvalTmr);
//           }
//           $("#analysisListTabl").html(data.tablTempl);
//
//           $("#analysis-table").DataTable({
//             "columnDefs": [
//               { "type": "date-euro", targets: 2 }
//             ],
//             "order": [[ 2, "desc" ]]
//           });
        window.parent.document.getElementById('aux20CloseBtn').click();
      },
      error: function (request, status, error) {
        console.log("E: in writeConfigTextfile! something went wrong!!!")
      },
      complete: function () {
        console.log('ajax writeConfigTextfile completed');
      }
    });
    // $.ajax({
    //   url: "${createLink(controller: 'expFile', action: 'toggleFilter')}",
    //   dataType: "json",
    //   data: {"id": JSON.stringify(eId)},
    //   type: "get",
    /* success: function (data) {
      console.log('toggleFilter success!');
    },
    error: function (request, status, error) {
      alert(error);
    },*/
    // complete: function () {
    //   console.log('ajax completed');
    // }


    // });
  }


var dotSize = 2;

var chart1 = dc.scatterPlot("#pops1");
var chart2 = dc.scatterPlot("#pops2");
var chart3 = dc.scatterPlot("#pops3");
var chart4 = dc.scatterPlot("#pops4");
var chart5 = dc.scatterPlot("#pops5");

var acsVal = window.parent.document.getElementById('acs');
// var acs = JSON.parse(acsVal.value);
var acs = acsVal.value;
console.log('acs=',acs,typeof acs);
// var bcs= JSON.parse(acs.replace(/'/g, '"')+"]");
var bcs= JSON.parse(acs.replace(/'/g, '"'));
// if(acsVal){
//   acsVal.innerHTML = 'SUCCESS';
// }
var saveBtn = window.parent.document.getElementById('aux20SaveBtn');
saveBtn.onclick = saveBtnClicked;
// saveBtn.addEventListener("click", saveBtnClicked());
// saveBtn.addEventListener("click", function(){
// document.getElementById("demo").innerHTML = "Hello World";
// saveBtn.innerHTML = "Hello World";
// });
if(saveBtn){ saveBtn.style.display =''; }


// d3.csv("assets/merged_2000.csv").then( function(data) {
d3.csv("assets/merged_1562.csv").then( function(data) {
  // d3.csv("assets/merged_1000.csv").then( function(data) {

  var pops = crossfilter(data),
    dim1 = pops.dimension(function (d) {
      return [+d.APC_CD14, +d.SSC_A, +d.pop6];
    }),
    dim2 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_Cy7_CD19, +d.pop7];
    }),
    dim3 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_CD56, +d.pop8];
    }),
    dim4 = pops.dimension(function (d) {
      return [+d.APC_eF780_CD4, +d.BV650_CD8a, +d.pop12];
    }),
    dim5 = pops.dimension(function (d) {
      return [+d.APC_eF780_CD4, +d.FITC_CD25, +d.pop14];
    }),

    group1 = dim1.group(),
    group2 = dim2.group(),
    group3 = dim3.group(),
    group4 = dim4.group(),
    group5 = dim5.group();


  var plotColorMap = {0: '#e51a22', 1: '#1e30ff'};

  chart1
    .width(300)
    .height(300)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .on('renderlet', function(chart1) {
      if(!chart1.select('rect.background').size())
        chart1.svg()
          .insert('rect', ':first-child')
          .attr('class', 'background')
          // .attr({width: chart1.xAxisLength(), height: chart1.yAxisHeight()})
          .attr({width: 300, height: 300})
          // .attr('transform', 'translate(' + chart1.margins().left + ', ' + chart1.margins().top + ')')
          // .style({fill: 'lightgreen'});
          // .style('fill','lightgreen');
          .style('fill','lightgreen');
    })
    // .xAxisLabel("APC_CD14")
    .xAxisLabel(bcs[0])
    // .yAxisLabel("SSC-A")
    .yAxisLabel(bcs[1])
    .clipPadding(10)
    .dimension(dim1)
    .symbolSize(dotSize)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    /*
    .on('filtered', function(chart) {
      var filters = chart.filters();
      if(filters.length) {
        var range = filters[0];
        console.log('range:', range[0], range[1]);
      }
      else
        console.log('no filters');
    })
    */
    // .colors(function(d) {console.log('colD',d.pop6); return 'red'})
    .group(group1);
  chart1.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart1.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart2
    .width(300)
    .height(300)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_Cy7_CD19")
    .clipPadding(10)
    .dimension(dim2)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group2);
  chart2.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart2.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart3
    .width(300)
    .height(300)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_CD56")
    .clipPadding(10)
    .dimension(dim3)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group3);
  chart3.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart3.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart4
    .width(300)
    .height(300)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_eF780_CD4")
    .yAxisLabel("BV650_CD8a")
    .clipPadding(10)
    .dimension(dim4)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group4);
  chart4.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart4.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart5
    .width(300)
    .height(300)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .yAxisLabel("FITC_CD25")
    .xAxisLabel("APC_eF780_CD4")
    .clipPadding(10)
    .dimension(dim5)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group5);
  chart5.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart5.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  dc.renderAll();
});
