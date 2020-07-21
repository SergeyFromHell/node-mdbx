function createDeferred() {
	const result = {};
	result.promise = new Promise((resolve,reject) => {
		result.resolve = resolve;
		result.reject = reject;
	});
	result.promise.catch(() => {});
	return result;
}

exports = module.exports = createDeferred;