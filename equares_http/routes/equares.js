// equares.js
var child_process = require('child_process');
var lins = require('line-input-stream');
var stream = require('stream');
var url = require("url");

var equares = {};
equares.cwd = "public/equares-cwd"

equares.programPath = (function() {
    var path = process.env["EQUARES_BIN"];
    if( path == undefined ) {
        err = 'EQUARES_BIN environment variable is missing';
        console.log( err );
        throw err;
        }
    return path + '/equares_con';
    })();

function StreamLineHistory(str, ls) {
    this.stream = str;  // Input/output stream
    this.lstream = ls;  // Line input stream
    this.history = [];
    }


function User(name) {
    this.name = name;
    this.proc = undefined;
    this.lastev = 0;
    this.stdio = [];    // Array for server process streams
}

equares.users = {};
equares.user = function( name ) {
    if( equares.users[name] === undefined )
        equares.users[name] = new User(name);
    return equares.users[name];
}

User.prototype.isRunning = function() {
    return this.proc !== undefined;
}

User.prototype.addStreamLine = function(code, line) {
    var h = this.stdio[code].history;
    h[h.length] = line;
}

User.prototype.initStream = function(code, str, optConsumer) {
    //console.log("initStream: consumer=", optConsumer);
    var user = this;
    var ls = lins(str);
    user.stdio[code] = new StreamLineHistory(str, ls);
    ls.setEncoding("utf8");
    ls.setDelimiter("\n");
    ls.on('error', function(err) {
        console.log(err + ", user " + user.name);
    });
    ls.on('line', function(line) {
        var ch = code == 0 ?   "=>" :   code == 1 ?   "<=" :   "):";
        console.log("%s@equares %s %s", user.name, ch, line);
        user.addStreamLine(code, line);
        if(optConsumer !== undefined)
            optConsumer.write(line+'\n');
    });
}

User.prototype.stopped = function() {
    this.proc = undefined;
    this.lastev = 0;
    this.stdio = [];
}

User.prototype.start = function() {
    var user = this;
    console.log( 'Starting equares for user %s', user.name );
    this.proc = child_process.spawn(equares.programPath, ['-s'], {cwd: equares.cwd});
    this.lastev = 1;
    this.proc.on('close', function() {
        console.log( 'equares for user %s has been closed', user.name );
        user.stopped();
    });
    this.proc.on('error', function() {
        console.log( 'equares for user %s terminated unexpectedly', user.name );
        user.stopped();
    });
    // console.log("start: user.proc.stdin = ", user.proc.stdin);
    user.initStream(0, new stream.PassThrough(), user.proc.stdin);
    user.initStream(1, user.proc.stdout);
    user.initStream(2, user.proc.stderr);
}

User.prototype.execCommand = function(command) {
    if( this.proc )
        this.stdio[0].stream.write(command + "\n");
}

User.prototype.stop = function() {
    if( this.proc ) {
        console.log( 'Stopping equares for user %s', this.name );
        this.proc.kill();

        // Do this here (in addition to 'stopped' handler)
        // in order isRunning to work correctly before stopped() get called.
        this.proc = undefined;
    }
}

User.prototype.toggle = function() {
    if (this.isRunning())
        this.stop();
    else
        this.start();
};

var commands = exports.commands = {}

commands["equaresToggle"] = function(request, response) {
    var user = equares.user("x");
    user.toggle();
    response.end();
};

commands["equaresStat"] = function(request, response) {
    var user = equares.user("x");
    response.send(user.isRunning()? "1": "0");
};

commands["equaresStatEvent"] = function(request, response) {
    response.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });
    response.write('\n');
    var user = equares.user("x");
    var goon = true;
    response.on('error', function() {
        // console.log("equaresStatEvent: stopping due to error");
        goon = false;
        });
    response.on('close', function() {
        // console.log("equaresStatEvent: stopping due to close");
        goon = false;
        });
    var lastev;
    (function checkStatEvents() {
        if( !goon )
            return;
        if( lastev !== user.lastev ) {
            lastev = user.lastev;
            response.write("data: " + lastev + "\n\n");
            // console.log("equaresStatEvent: %s", lastev);
            }
        setTimeout( checkStatEvents, 200 );
    })();
};

commands["equaresOutputEvent"] = function(request, response) {
    response.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });
    response.write('\n');
    var user = equares.user("x");
    var goon = true;
    var nstreams = 3;// user.stdio.length;
    var lines = [];
    for( var i=0; i<nstreams; ++i )
        lines[i] = 0;
    // console.log("=== equaresOutputEvent ===");
    response.on('error', function() {
        // console.log("equaresOutputEvent: stopping due to error");
        goon = false;
        });
    response.on('close', function() {
        // console.log("equaresOutputEvent: stopping due to close");
        goon = false;
        });
    (function checkOutputEvents() {
        if( !goon )
            return;
        var haveEvent = false;
        for( var i=0; i<nstreams; ++i ) {
            if( user.stdio[i] == undefined ) {
                lines[i] = 0;
                continue;
            }
            var a = user.stdio[i].history;
            if( lines[i] < a.length ) {
                response.write("data: " + JSON.stringify({stream: i, text: a[lines[i]]}) + "\n\n");
                // console.log("equaresOutputEvent[%d]: %s", i, a[lines[i]]);
                ++lines[i];
                haveEvent = true;
            }
        }
        setTimeout( checkOutputEvents, haveEvent? 0: 200 );
    })();
};

commands["equaresRunSimulation"] = function(request, response) {
    var user = equares.user("x");
    function startSim() {
        // Start server
        user.start();

        // Feed input
        var simulation = url.parse(request.url, true).query.simulation
        var command = "===={\n" + "runSimulation(\n" + simulation + "\n)\n" + "====}"
        user.execCommand(command);
        response.send("Started simulation");
    }

    if (user.isRunning()) {
        // Run simulation after stopping the currently running server instance
        user.proc.on('close', startSim);
        user.stop();
    }
    else
        startSim();
}

commands["equaresExec"] = function(request, response) {
    var user = equares.user("x");
    var command = url.parse(request.url, true).query.cmd;
    user.execCommand(command);
    response.send(user.isRunning(user)? "1": "0");
};

commands["equaresExecSync"] = function(request, response) {
    var user = equares.user("x");
    if (!user.isRunning())
    {
        // Failed to execute command - server is not running
        response.write(JSON.stringify({running: false, text: ""}));
        response.end();
        return;
    }

    var ls = user.stdio[1].lstream; // Line-wise stdout of server process
    var replyStarted = false;
    var replyLines = [];
    var processServerReply = function(line) {
        line = line.trim(); // Remove trailing \r
        if (!line.match(/==1==> /))
            return;
        line = line.substr(7);
        if (!replyStarted) {
            replyStarted = line === "===={";
            return;
        }
        if (line === "====}") {
            ls.removeListener('line', processServerReply);
            response.write(JSON.stringify({running: true, text: replyLines.join('\n')}));
            response.end();
        }
        else
            replyLines.push(line);
    }
    ls.on('line', processServerReply);

    var command = url.parse(request.url, true).query.cmd;
    user.execCommand(command);
};

var equaresInfoCache = {};

commands["equaresRequestInfo"] = function(request, response) {
    var command = url.parse(request.url, true).query.cmd;
    if (equaresInfoCache[command]) {
        response.write(equaresInfoCache[command]);
        response.end();
    }
    else {
        child_process.exec(equares.programPath + " -d " + command, {cwd: equares.cwd}, function (error, stdout, stderr) {
            var result = {
                stdout: stdout,
                stderr: stderr
            }
            if (error)
                result.error = error;
            response.write(equaresInfoCache[command] = JSON.stringify(result));
            response.end();
        });
    }
}

commands["equaresRequestInfoEx"] = function(request, response) {
    var query = url.parse(request.url, true).query
    var describeOptions = query.options || "";
    var command = query.cmd;
    var proc = child_process.spawn(equares.programPath, ['-i', '-d'+describeOptions, command], {cwd: equares.cwd});
    var stdout = "", stderr = "", replied = false
    function reply(text) {
        if (!replied) {
            replied = true
            response.write(text)
            response.end()
        }
    }

    proc.stdout.on("data", function(chunk) {
        stdout += chunk.toString()
    })
    proc.stderr.on("data", function(chunk) {
        stderr += chunk.toString()
    })
    proc.stdin.on('error', function(){
        reply(JSON.stringify({error: -1, message: "Failed to start equares"}))
    })
    proc.stdin.end(query.stdin)
    proc.on('close', function(code) {
        if (code === 0   &&   stderr.length === 0)
            reply(JSON.stringify(stdout))
        else
            reply(JSON.stringify({error: code, stdout: stdout, stderr: stderr}))
    });
    proc.on('error', function() {
        if (stderr.length > 0   ||   stdout.length > 0)
            // Process has started and returned a nonzero error code.
            // This means that 'close' will follow - we will reply there.
            // Note: Could not deduce reason for error from args, they are empty :(
            return;
        reply(JSON.stringify({error: -1, message: "Failed to start equares"}))
    });
}

exports.bind = function(app) {
    for (var cmd in commands)
        app.get("/" + cmd + ".cmd", commands[cmd])
}
