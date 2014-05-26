module.exports = function(app) {
    app.get('/doc', function(req, res) {
        res.render('doc')
    })
}
