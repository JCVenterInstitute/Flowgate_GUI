

// global vars
var points = []; //, g;
var dataset;
var jsonData;
var dragging = false, drawing = false, startPoint;
var margin = {top: 20, right: 20, bottom: 30, left: 140},
  width = 500, // + margin.left + margin.right,
  height = 500, // + margin.top + margin.bottom,
  // radius = 1;
  radius = 0.5;

d3.selection.prototype.moveToFront = function() {
  return this.each(function(){
    this.parentNode.appendChild(this);
  });
};

d3.selection.prototype.moveToBack = function() {
  return this.each(function() {
    var firstChild = this.parentNode.firstChild;
    if (firstChild) {
      this.parentNode.insertBefore(this, firstChild);
    }
  });
};


// behaviors
var dragger = d3.behavior.drag()
  .on('drag', handleDrag)
  .on('dragend', function(d){
    dragging = false;
  });

d3.json("d3data", function(error, data){
  if(error) throw error;
  console.log(data);
  if(data){
    jsonData = JSON.parse(data.jsonFile);
    dataset = data.dataset;
    console.log('jsonDat ', jsonData);
    doDraw(jsonData);
  }
// }, 'jsonp');
});


var svg = d3.select('body').append('svg')
  .attr("width", width+margin.left+margin.right  )
  .attr("height", height+margin.bottom+margin.top);
  // .append("g")
  // .attr("class", "mycanvas");
  // .attr("transform", "translate(" + width + ", " + height + ")");
//   .attr('transform', 'translate(' + margin.left + ',' + margin.top +')');

svg.append('rect')
  .attr('class','graph-background')
  .attr('x',margin.left)
  .attr('y', margin.top)
  .attr('width', width)
  .attr('height', height-margin.top);
  // .attr('fill','#ddd')
  // .attr('stroke', '#ddd');

var xScale = d3.scale.linear()
  // .domain([0, d3.max(jsonData.datap, function (d) { return d.x + 10; })])
  .domain([0, 4106])
  .range([margin.left, margin.left+width]);  // Set margins for x specific

var yScale = d3.scale.linear()
  // .domain([0, d3.max(jsonData.datap, function (d) { return d.y + 10; })])
  .domain([0, 4096])
  .range([height, margin.top]);  // Set margins for y specific

// Axes
var xAxis = d3.svg.axis().scale(xScale).tickValues([0,1024,2048,3072,4096]).orient("bottom");
var yAxis = d3.svg.axis().scale(yScale).tickValues([0,1024,2048,3072,4096]).orient("left");

// should get the color information from server
function colorCirc(data){
  if(data === 1) {return '#ffffff'; } else { return '#e51a22'; }
}

// function revXScale(data){
//   return data-margin.left;
// }

// function revYScale(data){
//   return data;
//

function doDraw(datap) {

  var xmarker = jsonData.marker['x-title'];
  var ymarker = jsonData.marker['x-title'];

  var circleAttrs = {
    cx: function(d) { return xScale(d["AF700_CD3"]); },
    cy: function(d) { return yScale(d["PE_CD56"]); },
    fill: function(d) { return colorCirc(d["pop8"]) },
    stroke: function(d) { return colorCirc(d["pop8"]) },
    r: radius,
    class: 'dot'
  };


  svg.on("click", function() {
    var coords = d3.mouse(this);

    // Normally we go from data to pixels, but here we're doing pixels to data
    var newData= {
      x: Math.round( xScale.invert(coords[0])),  // Takes the pixel number to convert to number
      y: Math.round( yScale.invert(coords[1]))
    };

    dataset.push(newData);   // Push data to our array

    svg.selectAll("circle")  // For new circle, go through the update process
      // .data(dataset)
      .data(jsonData.datap)
      .enter()
      .append("circle")
      .attr(circleAttrs)  // Get attributes from circleAttrs var
      .attr('class', 'dot')
      .on("mouseover", handleMouseOver)
      .on("mouseout", handleMouseOut);
  });


  // Axes labels
  svg.append("text")
    .attr('class','x-axis-title')
    .attr('x', 50+(width/2))
    .attr('y', height+40)
    .text(xmarker);

  svg.append("text")
    .attr('class','y-axis-title')
    .attr('x', 20)
    .attr('y', height/2)
    .style('text-anchor','end')
    .attr('transform', 'rotate(-90,20,200)')
    .text(ymarker);


  // final uncommend
 /* */
  svg.selectAll("circle")
    // .data(dataset)
    .data(jsonData.datap)
    .enter()
    .append("circle")
    .attr(circleAttrs)  // Get attributes from circleAttrs var
    .on("mouseover", handleMouseOver)
    .on("mouseout", handleMouseOut);
  /* */


  // Adds X-Axis as a 'g' element
  svg.append("g").attr({
    "class": "xAxis",  // Give class so we can style it
    // transform: "translate(" + [0, margin.top] + ")"  // Translate just moves it down into position (or will be on top)
    // transform: "translate(" + [0, margin.top + height] + ")"  // Translate just moves it down into position (or will be on top)
    transform: "translate(" + [0, height] + ")"
  }).call(xAxis);  // Call the xAxis function on the group

  // Adds Y-Axis as a 'g' element
  svg.append("g").attr({
    "class": "yAxis",
    // transform: "translate(" + [margin.left, margin.top + margin.bottom] + ")"
    transform: "translate(" + [margin.left, margin.top-margin.top] + ")"  // Translate just moves it down into position (or will be on top)
      // .attr("transform", "translate(" + margin.left + "," +margin.top+")").call(yAxis);
  }).call(yAxis);  // Call the yAxis function on the group

  // svg.append("g").attr('class', 'gate').attr();


  var xstart = jsonData.gate['x-start'];
  var ystart = jsonData.gate['y-start'];

  var xend = jsonData.gate['x-end'];
  var yend = jsonData.gate['y-end'];

  svg.append('rect')
    .attr('class','gate')
    .attr('width',xScale(margin.left + xend - xstart ))
    .attr('height',yScale((ystart * 20.48) + margin.top ))
    .attr('x',xScale(xstart * 20.48))
    .attr('y',yScale(yend * 20.48))
    .attr('stroke',2)
    .attr('fill','none');
    // .moveToFront();


} //end of doDraw()


svg.on('mouseup', function(){
  console.log('svg mouseup');
  if(dragging) return;
  drawing = true;
  startPoint = [d3.mouse(this)[0], d3.mouse(this)[1]];
  // if(svg.select('g.drawPoly').empty()) g = svg.append('g').attr('class', 'drawPoly');
  if(svg.select('g.drawPoly').empty()) g = svg.append('g').attr('class', 'drawPoly');
  console.log(g);
  // if(svg.select('g.drawPoly').empty()) g = svg.append('g').attr('class', 'drawPoly').attr("transform", "translate(" + margin.left + " " + margin.top + ")");
  if(d3.event.target.hasAttribute('is-handle')) {
    closePolygon();
    return;
  }
  points.push(d3.mouse(this));
  g.select('polyline').remove();
  var polyline = g.append('polyline').attr('points', points)
    .style('fill', 'none')
    .attr('stroke', '#000');
  for(var i = 0; i < points.length; i++) {
    g.append('circle')
      .attr('cx', points[i][0])
      .attr('cy', points[i][1])
      .attr('r', 3)
      .attr('fill', 'yellow')
      .attr('stroke', '#000')
      .attr('is-handle', 'true')
      .style({cursor: 'pointer'});
  }
});

function closePolygon() {
  svg.select('g.drawPoly').remove();
  var g = svg.append('g');
  g.append('polygon')
    .attr('points', points)
    .style('fill', getRandomColor());
  for(var i = 0; i < points.length; i++) {
    console.log('poly points ',xScale.invert(points[i][0]), yScale.invert(points[i][1]));
    var circle = g.selectAll('circles')
      .data([points[i]])
      .enter()
      .append('circle')
      .attr('cx', points[i][0])
      .attr('cy', points[i][1])
      .attr('r', 3)
      .attr('fill', '#FDBC07')
      .attr('stroke', '#000')
      .attr('is-handle', 'true')
      .style({cursor: 'move'})
      .call(dragger);
  }
  points.splice(0);
  drawing = false;
}

svg.on('mousemove', function() {
  if(!drawing) return;
  var g = d3.select('g.drawPoly');
  g.select('line').remove();
  var line = g.append('line')
    .attr('x1', startPoint[0])
    .attr('y1', startPoint[1])
    .attr('x2', d3.mouse(this)[0] + 2)
    .attr('y2', d3.mouse(this)[1])
    .attr('stroke', '#53DBF3')
    .attr('stroke-width', 3);
});


function handleDrag() {
  if(drawing) return;
  var dragCircle = d3.select(this), newPoints = [], circle;
  dragging = true;
  var poly = d3.select(this.parentNode).select('polygon');
  var circles = d3.select(this.parentNode).selectAll('circle');
  dragCircle
    .attr('cx', d3.event.x)
    .attr('cy', d3.event.y);
  for (var i = 0; i < circles[0].length; i++) {
    circle = d3.select(circles[0][i]);
    newPoints.push([circle.attr('cx'), circle.attr('cy')]);
  }
  poly.attr('points', newPoints);
}


function getRandomColor() {
  console.log('polygonClosed getRandomColor');
  var letters = '0123456789ABCDEF'.split('');
  var color = '#';
  for (var i = 0; i < 6; i++) {
    color += letters[Math.floor(Math.random() * 16)];
  }
  return color;
}


/*
var x = d3.scale.linear()
  .range([margin.left, width]);
console.log('axis x', x);

var y = d3.scale.linear()
  .range([height-margin.top, margin.left]);
console.log('axis y', y);

var xAxis = d3.svg.axis()
  .scale(x)
  .orient("bottom");

var yAxis = d3.svg.axis()
  .scale(y)
  .orient("left");

var xg = svg.append("g")
  .attr("class", "x axis")
  .attr("transform", "translate(0," + height + ")").call(xAxis);

var yg = svg.append("g")
  .attr("class", "y axis")
  .attr("transform", "translate(" + margin.left + "," +margin.top+")").call(yAxis);
*/


// Adds X-Axis as a 'g' element
// svg.append("g").attr({
//   "class": "axis",  // Give class so we can style it
//   transform: "translate(" + [0, margin.top] + ")"  // Translate just moves it down into position (or will be on top)
// }).call(xAxis);  // Call the xAxis function on the group
//

// On Click, we want to add data to the array and chart
/*
svg.on("click", function() {
  var coords = d3.mouse(this);

  // Normally we go from data to pixels, but here we're doing pixels to data
  var newData= {
    x: Math.round( xScale.invert(coords[0])),  // Takes the pixel number to convert to number
    y: Math.round( yScale.invert(coords[1]))
  };

  dataset.push(newData);   // Push data to our array

  svg.selectAll("circle")  // For new circle, go through the update process
    .data(dataset)
    .enter()
    .append("circle")
    .attr(circleAttrs)  // Get attributes from circleAttrs var
    .on("mouseover", handleMouseOver)
    .on("mouseout", handleMouseOut);
});
*/

// Create Event Handlers for mouse
function handleMouseOver(d, i) {  // Add interactivity

  // Use D3 to select element, change color and size
  d3.select(this).attr({
    fill: "orange",
    r: radius * 2
  });

  // Specify where to put label of text
  svg.append("text").attr({
    id: "t" + d.x + "-" + d.y + "-" + i,  // Create an id for text so we can select it later for removing on mouseout
    x: function() { return xScale(d.x) - 30; },
    y: function() { return yScale(d.y) - 15; }
  })
    .text(function() {
      return [d.x, d.y];  // Value of the text
    });
}

function handleMouseOut(d, i) {
  // Use D3 to select element, change color back to normal
  d3.select(this).attr({
    fill: "black",
    r: radius
  });

  // Select text by id and then remove
  d3.select("#t" + d.x + "-" + d.y + "-" + i).remove();  // Remove text location
}
