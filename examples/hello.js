'use strict';

let greetings = [
  'Hello World!',
  'Servus!',
  'Hallo!'
];
for (let greeting of greetings) {
  process.stdout.write(`${greeting}\n`);
}

// let arr = [ 1, 2, 3, 4 ];

// (function() {
//   log(arr.map((x) => x * x));
// })();
// var stdout = new Writeable.default();

// log(typeof Stream.Writeable);
// var stdout = new Stream.Writeable({
//   start() {},
//   write() {}
// });

// log('' + [ 1, 2, 3, 4 ].map(x => x * x));

// let obj = {
//   squares: squares
// };
// obj.squares();

// class Doubler {
//   constructor(n) {
//     this.n = n;
//   }

//   double() { return this.n * 2; }
// }

// function twice(x) {
//   let result = new Doubler(x).double();
//   log(`twice(${x}) = ${result}`);
//   return result;
// }

// let obj = {
//   squares() { return [ 1, 2, 3, 4 ].map(x => x); }
// };
// obj.squares();

// if (true) {
//   const letters = ['a', 'b', 'c'];
//   for (let letter of letters) {
//     log('letter: ' + letter);
//   }
// }
// log('Block-scoping: ' + typeof letters);

// setTimeout(function() {
//   twice(process.pid + 1);
// }, 200);

// setTimeout(function() {
//   twice(process.pid);
//   setTimeout(function() {
//     log('Surprise!');
//   }, 1000);
// }, 100);

// export Doubler;

'done';
