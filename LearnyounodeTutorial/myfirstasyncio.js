var fs = require('fs') // require is a special function provided by node
var myNumber = undefined // we don't know what the number is yet since it is stored in a file

function addOne(callback) {
  fs.readFile(process.argv[2], function doneReading(err, fileContents) {
    var contents = fs.readFileSync(process.argv[2])
    var lines = contents.toString().split('\n').length - 1
    callback(lines)
  })
}

function getLineCount(lines) {
	console.log(lines)
}

addOne(getLineCount)