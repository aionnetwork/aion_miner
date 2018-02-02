# equihashverify
nodejs native binding to check for valid Equihash solutions

## usage:
````javascript
var ev = require('bindings')('equihashverify.node');

var header = new Buffer(..., 'hex');
var solution = new Buffer(..., 'hex'); //do not include byte size preamble "fd4005"

ev.verify(header, solution);
//returns boolean
````

The header format must be 508 bytes long split between 476 bytes containing all header fields except the nonce + 32 byte nonce.
The solution format must be in the compressed format; 1344 bytes for parameters 2xx,9.
