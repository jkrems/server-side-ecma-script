(function(process) {
  'use strict';

  var global = this.global = this;
  var binding = process.binding;
  global.process = process;
  process.pid = 42;

  var script = binding('script');
  var Timers = binding('timers');
  var delay = Timers.delay;

  function noop() {}

  function setupTimers() {
    global.setTimeout = function setTimeout(fn, msec) {
      var delayed = delay(msec);
      delayed.then(fn, noop);
      return delayed;
    };
  }

  function setupStdout() {
    var tty = binding('tty');
    var sink = new tty.TTY(1);
    for (var method in sink) {
      sink[method] = sink[method].bind(sink);
    }
    var stdout = process.stdout = new Stream.Writeable(sink);
  }

  setupTimers();

  script.runFile('lib/stream.js');
  setupStdout();
});
