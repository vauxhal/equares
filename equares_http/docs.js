var fs = require('fs')
var boxInfo = require('./routes/equares').boxInfo

function PortFormat(port)
{
     this.format = port.format
     this.hints = port.hints
}

PortFormat.prototype.toHtml = function() {
    if (this.format instanceof Array) {
        var text = ''
        var count = 1
        for (var i=0; i<this.format.length; ++i) {
            if (i > 0)
                text += ' x '
            text += this.format[i]
            count *= this.format[i]
        }
        text += count == 1? ' element': ' elements'
        if (this.hints) {
            text += ' ('
            for (i=0; i<this.hints.length; ++i) {
                if (i > 0)
                    text += ', '
                text += '<i>' + this.hints[i] + '</i>'
            }
            text += ')'
        }
        return text
    }
    else
        return 'unspecified'
}

module.exports = function(app) {
    app.get('/doc', function(req, res) {
        res.render('doc')
    })
    app.get('/doc-menu-pane', function(req, res) {
        boxInfo(req, 'boxTypes', function(box, info) {
            info = eval(JSON.parse(info).stdout)
            res.render('doc-menu-pane', {boxTypes: info})
        })
    })

    function boxItemHelpText(item, caption) {
        if (item === undefined)
            return ''
        var text = '<h3>' + caption + '</h3>'
        if (item.length == 0)
            text += 'None.'
        else {
            text += '<ul>'
            for (var index in item) {
                var p = item[index]
                text += '<li>'
                text += '<b>' + p.name + '</b>'
                if (p.help)
                    text += '<br/>' + p.help
                if (p.format) {
                    var f = new PortFormat(p)
                    text += '<br/>Format: ' + f.toHtml()
                }
                text += '</li>'
            }
            text += '</ul>'
        }
        return text
    }

    function boxHelpText(box, info)
    {
        var text = '<h2>Synopsis</h2>'
        text += box + ' &mdash; '
        text += (info.help? info.help: 'No help available') + '<br>'
        text += boxItemHelpText(info.inputs, 'Input ports')
        text += boxItemHelpText(info.outputs, 'Output ports')
        text += boxItemHelpText(info.properties, 'Fixed parameters')
        return text
    }

    app.use('/doc/box', function(req, res, next) {
        var box = req.path.substr(1)
        boxInfo(req, box, function(box, info) {
            info = JSON.parse(info).stdout
            if (info)
                info = eval('(function(){return ' + info + '})()')
            else
                info = '<h2>N/A</h2>'
            var helpFile = 'public/meta/doc/box' + req.path + '.md'
            var filedata
            try {
                filedata = fs.readFileSync(helpFile, {encoding: 'utf8'})
            } catch(e) {
                filedata = 'TODO: Provide additional content in file ' + helpFile
            }
            res.send('<h1>' + box + '</h1>' + boxHelpText(box, info) + '<h2>Detailed description</h2>\n\n' + filedata)
        })
    })
    app.use('/doc/page', function(req, res, next) {
        res.redirect('/meta/doc/page' + req.path + '.md')
    })
}
