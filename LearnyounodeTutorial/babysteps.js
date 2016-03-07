var argSum = 0;
for(var i = 2; i < process.argv.length; i++) {
	argSum += Number(process.argv[i]);
}
console.log(argSum);
