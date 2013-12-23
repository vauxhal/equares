var http = require('http');
var fs = require('fs');
var url = require("url");

function respondmsg(msg, response)
{
    response.writeHead(200, {'Content-Type': 'text/plain'});
    response.end(msg + '\n');
}

function serveFile(pathname, request, response)
{
    var match = pathname.match(/\.([a-z]+)$/);
    if (!match)
        // No filename extension has been specified
        return false;
    var type = match[1];
    var contentType = serveFile.fileTypes[type];
    if (typeof(contentType) != 'string')
        // Unknown file type
        return false;
        
    var rs = fs.createReadStream('.' + pathname);
    rs.on('open', function(fd) {
        response.writeHead(200, {'Content-Type': contentType});
        rs.pipe(response);
    });
    rs.on('error', function() {
        respondmsg('404: Page ' + pathname + ' could not be found', response);
    });
    return true;
}
serveFile.fileTypes = {};
serveFile.fileTypes["html"] = "text/html";
serveFile.fileTypes["css"] = "text/css";
serveFile.fileTypes["js"] = "application/javascript";
serveFile.fileTypes["txt"] = "text/plain";
serveFile.fileTypes["png"] = "image/png";

function serveCommand(pathname, request, response)
{
    var cmd = pathname.substr(1, pathname.length-5);
    var handler = serveCommand.commands[cmd];
    if (typeof(handler) == 'function')
        handler(request, response);
    else
        respondmsg('Unable to find command "' + cmd + '"', response);
}
serveCommand.commands = {};

function serve(request, response)
{
    var pathname = url.parse(request.url).pathname;
    if( pathname == "/" )
        pathname += "index.html";
    if( /.*\.cmd$/.test(pathname) )
        serveCommand(pathname, request, response);
    else if (!serveFile(pathname, request, response)) {
        respondmsg('Don\'t know how to serve page "' + pathname + '"', response);
    }
}

exports.start = function() {
    http.createServer(function (request, response) {
        serve(request, response);
    }).listen(8000);
    console.log('Server running at http://127.0.0.1:8000/');
    };
exports.commands = serveCommand.commands;
exports.fileTypes = serveFile.fileTypes;
exports.respondmsg = respondmsg;
