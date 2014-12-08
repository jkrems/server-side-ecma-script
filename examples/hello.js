'use strict';

class Doubler {
  constructor(n) {
    this.n = n;
  }

  double() { return this.n * 2; }
}

function twice(x) {
  let result = new Doubler(x).double();
  log(`twice(${x}) = ${result}`);
  return result;
}

// let obj = {
//   squares() { return [ 1, 2, 3, 4 ].map(x => x); }
// };
// obj.squares();

if (true) {
  const letters = ['a', 'b', 'c'];
  for (let letter of letters) {
    log('letter: ' + letter);
  }
}
log('Block-scoping: ' + typeof letters);

setTimeout(function() {
  twice(process.pid + 1);
}, 200);

setTimeout(function() {
  twice(process.pid);
  setTimeout(function() {
    log('Surprise!');
  }, 1000);
}, 100);

'done';
