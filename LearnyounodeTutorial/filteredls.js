var fs = require('fs')
var filePath = process.argv[2]
var fileType = process.argv[3]
console.log(filePath)
console.log(fileType)

fs.readdir((filePath, function (err, list) {
	var lines = contents.toString().split('\n')
	console.log(lines)
}))