<!DOCTYPE html>
<style>


  line {
    stroke: red;
  }

  body {
    margin: 0;
    padding: 0;
    font: 12px sans-serif;
  }

  /*
  path,
  line {
    shape-rendering: crispEdges;
  }
  */

  path {
    shape-rendering: crispEdges;
  }

  /*
  svg,
  canvas {
    position: absolute;
  }
  */

  div,
  svg,
  canvas {
    position: absolute;
  }
  /*
  svg {
    pointer-events: none;
  }
  */

  .xAxis path,
  .yAxis path,
  .xAxis line,
  .yAxis line {
    fill: none;
    stroke: #000;
    stroke-width: 1px;
  }

  /*
  circle {
    stroke-width: 1px;
    stroke: #5c5c5c;
    !*stroke: #0efff8;*!
    fill: #ac2925;
  }
  */

  .dot {
    stroke-width: 1px;
    /*stroke: #ffffff;*/
    /*fill: #ac2925;*/
    fill: none;
  }

  .gate {
    stroke-width: 1px;
    stroke: #0f0;
    /*fill: #ac2925;*/
    fill: none;
    z-index: 100;
  }

  .graph-background {
    fill: #ddd;
    stroke: #ddd;
  }

  /*
  rect {
    stroke-width: 2px;
    stroke: #0a0;
    !*fill: #00e;*!
    fill: none;
  }
  */

  .hidden {
    display: none;
  }

  /*
  body {
    font: 11px sans-serif;
  }

  .axis path,
  .axis line {
    fill: none;
    stroke: #000;
    shape-rendering: crispEdges;
  }

  .dot {
    stroke: #000;
  }

  .tooltip {
    position: absolute;
    width: 200px;
    height: 28px;
    pointer-events: none;
  }
  */

  .mycanvas{
    z-index: 100;
  }

</style>
<head>
  <meta charset="utf-8">

  %{--<asset:javascript src="d3/d3.v57.min.js"/>--}%
  <asset:javascript src="d3/d3.v3517.min.js"/>
</head>
<body>

  %{--<svg width="960" height="600"></svg>--}%
  %{--<canvas width="960" height="600"></canvas>--}%
  %{--<script src="//d3js.org/d3.v4.min.js"></script>--}%
%{--
    <script>


    </script>

--}%
  <asset:javascript src="d3script.js"/>
</body>
