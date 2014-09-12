function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  
  xhr.open(type, url);
  xhr.send();
}

function getHours(pos) {
  var url = 'http://hidden-depths-1085.herokuapp.com/times';
  var cur = {
    long: pos.coords.longitude,
    lat: pos.coords.latitude
  };
  var wdw = {
    mk: {
      long: -81.581194,
      lat: 28.416492,
      id: 0
    },
    epcot: {
      long: -81.549283,
      lat: 28.375417,
      id: 1
    },
    holly: {
      long: -81.558595,
      lat: 28.35852,
      id: 3
    },
    dak: {
      long: -81.590553,
      lat: 28.357749,
      id: 4
    },
    blizz: {
      long: -81.575011,
      lat: 28.352171,
      id: 6
    },
    typh: {
      long: -81.529500,
      lat: 28.366000,
      id: 5
    },
    dd: {
      long: -81.515000,
      lat: 28.370000,
      id: 11
    }
  };
  var dlr = {
    dl: {
      long: -117.919117,
      lat: 33.810639,
      id: 0
    },
    advent: {
      long: -117.920799,
      lat: 33.80552,
      id: 1
    }  
  };
  
  
  console.log('Your current position is: ' + cur.lat + ' / ' + cur.long);
  
  var near;
  if ( (cur.long - wdw.mk.long) < (cur.long - dlr.dl.long) ) {
    near = wdw;
  } else {
    near = dlr;
  }
  console.log( near.toString(), (cur.long - wdw.mk.long), (cur.long - dlr.dl.long) );
  
  var close = 100;
  if ( near === wdw ) {
    for(var park in wdw) {
      console.log(park);
      close = ( ( cur.long - wdw[park].long ) < close ? park : close);
      console.log(close);
    }
  } else {
    for(var parke in dlr) {
      console.log(parke);
      close = ( ( cur.long - wdw[parke].long ) < close ? parke : close );
      console.log(close);
    }
  }
  
  xhrRequest(url, 'GET', function(resText) {
    var json = JSON.parse(resText);
    var id = near[close].id;
    
    var park = json.wdw.hours[id].title;
    console.log('Park name is: ' + park);
    
    var open = json.wdw.hours[id].open;
        open = open.slice(0, open.indexOf(':'));
    console.log(park + ' will open at: ' + open);
    
    var until = json.wdw.hours[id].until;
        until = until.slice(0, until.indexOf(':'));
    console.log(park + ' will be open until: ' + until);
    
    var dict = {
      "KEY_PARK": park,
      "KEY_OPEN": open,
      "KEY_UNTIL": until
    };
    
    Pebble.sendAppMessage(dict, 
      function(e) {
        console.log("Hours info sent correctly.");
      },
      function(e) {
        console.log("Error sending the hours info, try again.");
      }
    );
    
  });
}

function locationFail(err) {
  console.log('Error getting location.', err);
}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    getHours,
    locationFail,
    {timeout: 15000, maximumAge: 6000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS is ready!');
    
    getLocation();
    
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  
  getLocation();
  
}); 