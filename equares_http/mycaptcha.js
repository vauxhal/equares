/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var child_process = require('child_process');
var programPath = (function() {
    var path = process.env["EQUARES_BIN"];
    if( path == undefined ) {
        err = 'EQUARES_BIN environment variable is missing';
        console.log( err );
        throw err;
        }
    return path + '/captchagen';
    })();


module.exports = function(options) {
    if (typeof(options) === "string")
        options = { url: options }
    else {
        options = options || {}
        options.url = options.url || "/captcha.png"
    }
    return function(req, res, next) {
        if(req.path !== options.url)
            return next();

        // Generate captcha text
        var text = ('' + Math.random()).substr(3, 6);

        // Generate captcha image
        child_process.exec(programPath + " -platform offscreen " + text, function (error, stdout, stderr) {
            if (error)
                throw error
            stdout = new Buffer(stdout, 'base64').toString('binary')
            res.writeHead(200, {
                'Content-Length': stdout.length,
                'Content-Type': 'image/png'
            });
            res.write(stdout, 'binary');
            req.session.captcha = text;
            res.end();
        });
    }
}
