// equares.js
var child_process = require('child_process');
var lins = require('line-input-stream');
var stream = require('stream');
var fs = require('fs')
var path = require('path')
var simulation = require('../simulation')

var equares = {};

var equaresCachePath = path.normalize(__dirname + "/../equares-cache")
fs.mkdir(equaresCachePath, function() {})

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


function User(name, auth) {
    this.name = name
    this.auth = auth
    this.proc = undefined
    this.lastev = 0
    this.stdio = []     // Array for server process streams
}

equares.users = {};
equares.user = function(req) {
    var auth = req.isAuthenticated()
    var name = auth? req.user.username: ""  // TODO: session id
    return equares.users[name] || (equares.users[name] = new User(name, auth))
}

User.prototype.runConfig = function(args) {
    function merge(a, b) {
        if (!(b instanceof Array))
            return a
        for (var n=b.length, i=0; i<n; ++i)
            a.push(b[i])
        return a
    }

    if (this.auth)
        return {
            cwd: 'users/' + this.name,
            args: merge(args, ['-c', equaresCachePath])
        }
    else
        return {
            cwd: equaresCachePath,
            args: merge(args, ['-b'])
        }
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
    var rc = this.runConfig(['-s'])
    this.proc = child_process.spawn(equares.programPath, rc.args, {cwd: rc.cwd});
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

var commands = {}

function ensureAuth(req, res) {
    if (!req.isAuthenticated()) {
        res.send(401, "You are not logged in")
        return false
    }
    return true
}


commands["toggle"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    var user = equares.user(req)
    user.toggle()
    res.end()
};

commands["stat"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    var user = equares.user(req)
    res.send(user.isRunning()? "1": "0")
};

commands["statEvent"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    res.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });
    res.write('\n');
    var user = equares.user(req)
    var goon = true;
    res.on('error', function() {
        // console.log("statEvent: stopping due to error");
        goon = false;
        });
    res.on('close', function() {
        // console.log("statEvent: stopping due to close");
        goon = false;
        });
    var lastev;
    (function checkStatEvents() {
        if( !goon )
            return;
        if( lastev !== user.lastev ) {
            lastev = user.lastev;
            res.write("data: " + lastev + "\n\n");
            // console.log("statEvent: %s", lastev);
            }
        setTimeout( checkStatEvents, 200 );
    })();
};

commands["outputEvent"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    res.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });
    res.write('\n');
    var user = equares.user(req)
    var goon = true;
    var nstreams = 3;// user.stdio.length;
    var lines = [];
    for( var i=0; i<nstreams; ++i )
        lines[i] = 0;
    // console.log("=== outputEvent ===");
    res.on('error', function() {
        // console.log("outputEvent: stopping due to error");
        goon = false;
        });
    res.on('close', function() {
        // console.log("outputEvent: stopping due to close");
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
                res.write("data: " + JSON.stringify({stream: i, text: a[lines[i]]}) + "\n\n");
                // console.log("outputEvent[%d]: %s", i, a[lines[i]]);
                ++lines[i];
                haveEvent = true;
            }
        }
        setTimeout( checkOutputEvents, haveEvent? 0: 200 );
    })();
};

commands["runSimulation"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    var user = equares.user(req)
    function startSim() {
        // Start server
        user.start();

        // Feed input
        var script = req.body.script,
            simulation = req.body.simulation
        var command = "===={\n" + script + "\nrunSimulation(\n" + JSON.stringify(simulation) + "\n)\n" + "====}"
        user.execCommand(command);
        res.send("Started simulation");
    }

    if (user.isRunning()) {
        // Run simulation after stopping the currently running server instance
        user.proc.on('close', startSim);
        user.stop();
    }
    else
        startSim();
}

commands["sync"] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    var user = equares.user(req)
    var command = req.query.cmd
    if (!command.match(/^==\d+==\<$/))
        res.send(403)
    else {
        user.execCommand(command)
        res.send(user.isRunning(user)? "1": "0")
    }
};

var equaresInfoCache = {};

commands["requestInfo"] = function(req, res) {
    var command = req.query.cmd;
    if (equaresInfoCache[command]) {
        res.write(equaresInfoCache[command]);
        res.end();
    }
    else {
        var rc = equares.user(req).runConfig([equares.programPath, '-d', command])
        child_process.exec(rc.args.join(' '), {cwd: rc.cwd}, function (error, stdout, stderr) {
            var result = {
                stdout: stdout,
                stderr: stderr
            }
            if (error)
                result.error = error;
            res.write(equaresInfoCache[command] = JSON.stringify(result));
            res.end();
        });
    }
}

commands["requestInfoEx"] = function(req, res) {
    var query = req.body
    var describeOptions = query.options || ""
    var rc = equares.user(req).runConfig(['-i', '-d'+describeOptions, 'box'])
    var stdin = 'box = new ' + query.type + '\n'
    for (var prop in query.props)
        stdin += 'box.' + prop + ' = ' + JSON.stringify(query.props[prop]) + '\n'
    var proc = child_process.spawn(equares.programPath, rc.args, {cwd: rc.cwd});
    var stdout = "", stderr = "", replied = false
    function reply(text) {
        if (!replied) {
            replied = true
            res.write(text)
            res.end()
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
    proc.stdin.end(stdin)
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

commands['quicksave'] = function(req, res) {
    req.session.simulation = req.body.simulation
    simulation.RecentSim.set(req)
    res.end()
}

commands['quickload'] = function(req, res) {
    function sessionRecentSim() {
        return req.session.simulation   ||
            JSON.stringify({name: '', description: '', info: '', script: '', public: false, definition: JSON.stringify({boxes: [], links: []})})
    }

    if (req.isAuthenticated()) {
        simulation.RecentSim.get(req, function(sim) {
            if (typeof sim !== "string")
                sim = sessionRecentSim()
            res.send(sim)
        })
    }
    else
        res.send(sessionRecentSim())
}

commands['savesim'] = function(req, res) {
    if (!ensureAuth(req, res))
        return
    var sim = JSON.parse(req.body.simulation)
    sim.date = new Date()
    sim.user = req.user.id
    if (typeof sim.public != 'boolean')
        sim.public = false

    function save() {
        simulation.Sim.upsert(sim, function(err, sim) {
            if (err) {
                console.log(err)
                res.send(500, 'Failed to save simulation')
            }
            else
                res.end()
        })
    }

    var overwrite = JSON.parse(req.body.overwrite)
    if (overwrite)
        save()
    else
        simulation.Sim.have(sim, function(err, count) {
            if (err)
                res.send(500)
            else if (count > 0)
                res.send(403)
            else
                save()
        })
}

module.exports = function() {
    return function(req, res, next) {
        var name = req.path.substr(1)
        var cmd = commands[name]
        if (cmd)
            cmd(req, res)
        else
            res.send(404, 'Command not found')
    }
}
