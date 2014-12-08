(function(process) {
  'use strict';

  var global = this.global = this;
  var binding = process.binding;
  global.process = process;
  process.pid = 42;

  function noop() {}

  function setupTimers() {
    var Timers = binding('timers');
    var delay = Timers.delay;

    global.setTimeout = function setTimeout(fn, msec) {
      var delayed = delay(msec);
      delayed.then(fn, noop);
      return delayed;
    };
  }

  setupTimers();
});
