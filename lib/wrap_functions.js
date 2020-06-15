function wrapFunctions(from, to) {
    for (let key of Object.getOwnPropertyNames(Object.getPrototypeOf(from))) {
        if (key == 'constructor')
            continue;
        const value = from[key];
        if (typeof(value) != 'function')
            continue;
        to[key] = function() {
            return value.apply(from, arguments);
        };
    }
}

module.exports = wrapFunctions;