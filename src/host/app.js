g = new Dygraph(
    document.getElementById("graph"),
    // For possible data formats, see http://dygraphs.com/data.html
    // The x-values could also be dates, e.g. "2012/03/15"
    `X,Y,Z
    1,0,3
    2,2,6
    3,4,8
    4,6,9
    5,8,9
    6,10,8
    7,12,6
    8,14,3`,
    {
      // options go here. See http://dygraphs.com/options.html
      legend: 'always',
      animatedZooms: true,
      title: 'dygraphs ' + Dygraph.VERSION + ' chart template'
    });
