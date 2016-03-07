var fs = require("fs");
var buf = fs.readFileSync(process.argv[2]);
var str = buf.toString();
var strArray = str.split("\n");
var counter = 0
for(var something in strArray) {
	counter++;
}
console.log(counter-1);