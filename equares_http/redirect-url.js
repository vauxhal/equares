module.exports = function(app, path, transform) {
    if (arguments.length == 2) {
        transform = path
        path = ''
    }
    else if (arguments.length !== 3)
        throw new Exception('redirect-url: invalid number of arguments, 2 or 3 were expected')
    if (!( app instanceof Object   &&   typeof path == 'string'   &&   transform instanceof Function ))
        throw new Exception('redirect-url: invalid arguments')
    app.use(path, function(req, res, next) {
        var url=transform(req.path)
        if (typeof url == 'string')
            res.redirect(path + url)
        else
            next()
    })
}
