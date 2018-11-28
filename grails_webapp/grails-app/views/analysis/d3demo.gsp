<!DOCTYPE html>
<html lang="en">
<head>
  <title>dc.js - Scatter Plot Brushing Example</title>
  <meta charset="UTF-8">

  %{--<link rel="stylesheet" type="text/css" href="../css/bootstrap.min.css">--}%
  <asset:stylesheet src="bootstrap.min.css"/>
  <asset:stylesheet src="dc.min.css"/>

  %{--<link rel="stylesheet" type="text/css" href="../css/dc.css"/>--}%
  <style>
    .dc-chart .brush rect.selection{
      fill: #ac2925;
      fill-opacity: .125;
    }
  </style>
</head>
<body>
<div class="container">
  %{--<script type="text/javascript" src="header.js"></script>--}%

  <p>Draw selected gates for config file</p>
  <div id="p1"></div>
  <div id="p2"></div>
  <div id="p3"></div>
  <div id="p4"></div>
  <div id="p5"></div>


  %{--<script type="text/javascript" src="../js/d3.js"></script> --}%%{-- v 5.4.0--}%
  <asset:javascript src="d3/d3.v540.js"/>
  %{--<asset:javascript src="d3/d3.v540.js"/>--}%
  %{--<script type="text/javascript" src="../js/crossfilter.js"></script> --}%%{-- ev v 6.2 --}%
  <asset:javascript src="crossfilter/crossfilter.v62.js"/>
  %{--<script type="text/javascript" src="../js/dc.js"></script> --}%%{-- v 3.0.8 --}%
  <asset:javascript src="dc/dc.v308.min.js"/>

  <script type="text/javascript">
    var chart1 = dc.scatterPlot("#p1");
    var chart2 = dc.scatterPlot("#p2");
    var chart3 = dc.scatterPlot("#p3");
    var chart4 = dc.scatterPlot("#p4");
    var chart5 = dc.scatterPlot("#p5");

    // var data = d3.csv('/assets/plotdata.csv', function(error, data){
    // var data = d3.csv('/assets/plotdata.csv');
    // d3.csv("/assets/plotdata.csv", function(data) {
    // d3.csv("/assets/plotdata.csv").then( function(data) {
    d3.csv("/assets/testdata_200.csv").then( function(data) {
        // if(datas) {

          // var dataP = d3.csvParse(data);
          console.log('dats=', data);
          // console.log('datP=', dataP);

          data.forEach(function (x) {
            console.log('x=', x);
            x.APC_CD14 = +x.APC_CD14;
            x.AF700_CD3 = +x.AF700_CD3;
            x.APC_eF780_CD4 = +x.APC_eF780_CD4;
            x.V500_LiveDead = +x.V500_LiveDead;
            x.FITC_CD25 = +x.FITC_CD25;
            x.PE_CD56 = +x.PE_CD56;
            x.PerCP_Cy55_CCR7 = +x.PerCP_Cy55_CCR7;
          });

        /* */
          var ndx = crossfilter(data),
            dim1 = ndx.dimension(function (d) {
              console.log('d=',d);
              return [+d.FITC_CD25, +d.V500_LiveDead];
            }),
            dim2 = ndx.dimension(function (d) {
              return [+d.FITC_CD25, +d.PE_CD56];
            }),
            dim3 = ndx.dimension(function (d) {
              return [+d.APC_CD14, +d.AF700_CD3];
            }),
            dim4 = ndx.dimension(function (d) {
              return [+d.AF700_CD3, +d.APC_eF780_CD4];
            }),
            dim5 = ndx.dimension(function (d) {
              return [+d.APC_eF780_CD4, +d.FITC_CD25];
            }),

            group1 = dim1.group(),
            group2 = dim2.group(),
            group3 = dim3.group(),
            group4 = dim4.group(),
            group5 = dim5.group();

          chart1
            .width(300)
            .height(300)
            // .xAxis().tickValues([0, 1024, 2048, 3072, 4096]) //does not work here
            .x(d3.scaleLinear().domain([0, 4096]))//.ticks(3)
            // .xAxis().tickValues([0, 100, 200, 4000])
            .yAxisLabel("V500_LiveDead") //y
            .xAxisLabel("FITC_CD25")//x
            .xUnits([0,1024,2048,3072,4096])
            .clipPadding(10)
            .dimension(dim1)
            .excludedOpacity(0.5)
            .symbolSize(2)
            // .xAxis().tickValues([0, 1024, 2048, 3072, 4096])
            .group(group1);
            // .yAxis().ticks(4);
            // .yAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart1.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart1.yAxis().tickValues([1024, 2048, 3072, 4096]);

          chart2
            .width(300)
            .height(300)
            .x(d3.scaleLinear().domain([0, 4096]))
            .yAxisLabel("PE_CD56")//z
            .xAxisLabel("FITC_CD25")//y
            .clipPadding(10)
            .dimension(dim2)
            .excludedColor('#ddd')
            .symbolSize(2)
            .group(group2);
            // .yAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart2.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart2.yAxis().tickValues([1024, 2048, 3072, 4096]);

          chart3
            .width(300)
            .height(300)
            .x(d3.scaleLinear().domain([0, 4096]))
            .yAxisLabel("AF700_CD3")//v
            .xAxisLabel("APC_CD14")//u
            .clipPadding(10)
            .dimension(dim3)
            .excludedColor('#ddd')
            .symbolSize(2)
            .group(group3);
            // .yAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart3.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart3.yAxis().tickValues([1024, 2048, 3072, 4096]);

          chart4
            .width(300)
            .height(300)
            .x(d3.scaleLinear().domain([0, 4096]))
            .yAxisLabel("AF700_CD3")//v
            .xAxisLabel("APC_eF780")//w
            .clipPadding(10)
            .dimension(dim4)
            .excludedColor('#ddd')
            .symbolSize(2)
            .group(group4);
            // .yAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart4.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart4.yAxis().tickValues([1024, 2048, 3072, 4096]);

          chart5
            .width(300)
            .height(300)
            .x(d3.scaleLinear().domain([0, 4096]))
            .yAxisLabel("FITC_CD25")//w
            .xAxisLabel("APC_eF780")//x
            .clipPadding(10)
            .dimension(dim5)
            .excludedColor('#ddd')
            .symbolSize(2)
            .group(group5);
            // .yAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart5.xAxis().tickValues([0, 1024, 2048, 3072, 4096]);
          chart5.yAxis().tickValues([1024, 2048, 3072, 4096]);

          // dc.renderAll();
          dc.renderAll();
        /* */
        // }
    });
  </script>

</div>
</body>
</html>
