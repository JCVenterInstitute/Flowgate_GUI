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
var chart6 = dc.scatterPlot("#pops6");
var chart7 = dc.scatterPlot("#pops7");
var chart8 = dc.scatterPlot("#pops8");
var chart9 = dc.scatterPlot("#pops9");
var chart10 = dc.scatterPlot("#pops10");
var chart11 = dc.scatterPlot("#pops11");
var chart12 = dc.scatterPlot("#pops12");
var chart13 = dc.scatterPlot("#pops13");
var chart14 = dc.scatterPlot("#pops14");
var chart15 = dc.scatterPlot("#pops15");
var chart16 = dc.scatterPlot("#pops16");
var chart17 = dc.scatterPlot("#pops17");
var chart18 = dc.scatterPlot("#pops18");
var chart19 = dc.scatterPlot("#pops19");
var chart20 = dc.scatterPlot("#pops20");
var chart21 = dc.scatterPlot("#pops21");
var chart22 = dc.scatterPlot("#pops22");

// var acsVal = window.parent.document.getElementById('acs');
// var acs = JSON.parse(acsVal.value);
// var acs = acsVal.value;
// console.log('acs=',acs,typeof acs);
// var bcs= JSON.parse(acs.replace(/'/g, '"')+"]");
// var bcs= JSON.parse(acs.replace(/'/g, '"'));
// if(acsVal){
//   acsVal.innerHTML = 'SUCCESS';
// }
var saveBtn = window.parent.document.getElementById('aux20SaveBtn');
// saveBtn.addEventListener("click", saveBtnClicked());
// saveBtn.addEventListener("click", function(){
// document.getElementById("demo").innerHTML = "Hello World";
// saveBtn.innerHTML = "Hello World";
// });
if(saveBtn){
  saveBtn.onclick = saveBtnClicked;
  saveBtn.style.display ='';
}


// // d3.csv("assets/merged_2000.csv").then( function(data) {
// // d3.csv("assets/merged_1000.csv").then( function(data) {
// d3.csv("assets/merged_1562.csv").then( function(data) {
d3.csv("assets/allData_2000.csv").then( function(data) {

  // console.log('data ',data);

  var pops = crossfilter(data),
    dim1 = pops.dimension(function (d) {
      return [+d.FSC_A, +d.SSC_A, +d.pop1];
    }),
    dim2 = pops.dimension(function (d) {
      return [+d.FSC_A, +d.FSC_H, +d.pop2];
    }),
    dim3 = pops.dimension(function (d) {
      return [+d.SSC_A, +d.SSC_H, +d.pop3];
    }),
    dim4 = pops.dimension(function (d) {
      return [+d.V500_LiveDead, +d.SSC_A, +d.pop4];
    }),
    dim5 = pops.dimension(function (d) {
      return [+d.APC_CD14, +d.SSC_A, +d.pop5];
    }),
    dim6 = pops.dimension(function (d) {
      return [+d.APC_CD14, +d.SSC_A, +d.pop6];
    }),
    dim7 = pops.dimension(function (d) {
      return [+d.APC_CD14, +d.PE_Cy7_CD19, +d.pop7];
    }),
    dim8 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_CD56, +d.pop8];
    }),
    dim9 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_CD56, +d.pop9];
    }),
    dim10 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_CD56, +d.pop10];
    }),
    dim11 = pops.dimension(function (d) {
      return [+d.AF700_CD3, +d.PE_CD56, +d.pop11];
    }),
    dim12 = pops.dimension(function (d) {
      return [+d.APC_eF780_CD4, +d.BV650_CD8a, +d.pop12];
    }),
    dim13 = pops.dimension(function (d) {
      return [+d.APC_eF780_CD4, +d.BV650_CD8a, +d.pop13];
    }),
    dim14 = pops.dimension(function (d) {
      return [+d.APC_eF780_CD4, +d.FITC_CD25, +d.pop14];
    }),
    dim15 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop15];
    }),
    dim16 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop16];
    }),
    dim17 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop17];
    }),
    dim18 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop18];
    }),
    dim19 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop19];
    }),
    dim20 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop20];
    }),
    dim21 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop21];
    }),
    dim22 = pops.dimension(function (d) {
      return [+d.eF450_CD45RA, +d.PerCP_Cy55_CCR7, +d.pop22];
    }),

    group1 = dim1.group(),
    group2 = dim2.group(),
    group3 = dim3.group(),
    group4 = dim4.group(),
    group5 = dim5.group(),
    group6 = dim6.group(),
    group7 = dim7.group(),
    group8 = dim8.group(),
    group9 = dim9.group(),
    group10 = dim10.group(),
    group11 = dim11.group(),
    group12 = dim12.group(),
    group13 = dim13.group(),
    group14 = dim14.group(),
    group15 = dim15.group(),
    group16 = dim16.group(),
    group17 = dim17.group(),
    group18 = dim18.group(),
    group19 = dim19.group(),
    group20 = dim20.group(),
    group21 = dim21.group(),
    group22 = dim22.group();

  console.log('dim', dim1);

  var plotColorMap = {0: '#e51a22', 1: '#1e30ff'};

  chart1
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    // .on('renderlet', function(chart1) {
    //   if(!chart1.select('rect.background').size())
    //     chart1.svg()
    //       .insert('rect', ':first-child')
    //       .attr('class', 'background')
          // .attr({width: chart1.xAxisLength(), height: chart1.yAxisHeight()})
          // .attr({width: 200, height: 200})
          // .attr('transform', 'translate(' + chart1.margins().left + ', ' + chart1.margins().top + ')')
          // .style({fill: 'lightgreen'});
          // .style('fill','lightgreen');
          // .style('fill','lightgreen');
    // })
    .xAxisLabel("FSC_A")
    // .xAxisLabel(bcs[0])
    .yAxisLabel("SSC-A")
    // .yAxisLabel(bcs[1])
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
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("FSC_A")
    .yAxisLabel("FSC_H")
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
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("SSC_A")
    .yAxisLabel("SSC_H")
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
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("V500_LiveDead")
    .yAxisLabel("SSC_A")
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
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_CD14")
    .yAxisLabel("SSC_A")
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

  chart6
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_CD14")
    .yAxisLabel("SSC_A")
    .clipPadding(10)
    .dimension(dim6)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group6);
  chart6.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart6.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart7
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_CD14")
    .yAxisLabel("PE_Cy7_CD19")
    .clipPadding(10)
    .dimension(dim7)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group7);
  chart7.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart7.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart8
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_CD56")
    .clipPadding(10)
    .dimension(dim8)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group8);
  chart8.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart8.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart9
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_CD56")
    .clipPadding(10)
    .dimension(dim9)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group9);
  chart9.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart9.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart10
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_CD56")
    .clipPadding(10)
    .dimension(dim10)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group10);
  chart10.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart10.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart11
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("AF700_CD3")
    .yAxisLabel("PE_CD56")
    .clipPadding(10)
    .dimension(dim11)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group11);
  chart11.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart11.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart12
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_eF780_CD4")
    .yAxisLabel("BV650_CD8a")
    .clipPadding(10)
    .dimension(dim12)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group12);
  chart12.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart12.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart13
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_eF780_CD4")
    .yAxisLabel("BV650_CD8a")
    .clipPadding(10)
    .dimension(dim13)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group13);
  chart13.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart13.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart14
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("APC_eF780_CD4")
    .yAxisLabel("FITC_CD25")
    .clipPadding(10)
    .dimension(dim14)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group14);
  chart14.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart14.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart15
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim15)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group15);
  chart15.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart15.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart16
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim16)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group16);
  chart16.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart16.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart17
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim17)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group17);
  chart17.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart17.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart18
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim18)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group18);
  chart18.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart18.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart19
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim19)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group19);
  chart19.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart19.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart20
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim20)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group20);
  chart20.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart20.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart21
    .width(200)
    .height(200)
    .margins({top:5, bottom:50, right: 5,left: 50})
    .x(d3.scaleLinear().domain([0, 4096]))
    .y(d3.scaleLinear().domain([0, 4096]))
    .xAxisLabel("eF450_CD45RA")
    .yAxisLabel("PerCP_Cy55_CCR7")
    .clipPadding(10)
    .dimension(dim21)
    .excludedSize(dotSize)
    .excludedColor('#bbb')
    .symbolSize(dotSize)
    .colorAccessor(function (d) { return d.key[2]; })
    .colors(function(colorKey) { return plotColorMap[colorKey]; })
    .group(group21);
  chart21.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
  chart21.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  chart22
      .width(200)
      .height(200)
      .margins({top:5, bottom:50, right: 5,left: 50})
      .x(d3.scaleLinear().domain([0, 4096]))
      .y(d3.scaleLinear().domain([0, 4096]))
      .xAxisLabel("eF450_CD45RA")
      .yAxisLabel("PerCP_Cy55_CCR7")
      .clipPadding(10)
      .dimension(dim22)
      .excludedSize(dotSize)
      .excludedColor('#bbb')
      .symbolSize(dotSize)
      .colorAccessor(function (d) { return d.key[2]; })
      .colors(function(colorKey) { return plotColorMap[colorKey]; })
      .group(group22);
    chart22.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
    chart22.yAxis().tickValues([0, 1024, 2048, 3072, 4096]);

  dc.renderAll();
});
