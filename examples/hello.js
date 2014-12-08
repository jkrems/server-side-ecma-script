function twice(x) {
  log('twice(' + x + ') = ' + x * 2);
  return x * 2;
}

var p1 = setTimeout(function() {
  twice(process.pid + 1);
}, 100);

var p2 = setTimeout(function() {
  twice(process.pid);
  setTimeout(function() {
    log('Surprise!');
  }, 1000);
}, 500);

'done';
