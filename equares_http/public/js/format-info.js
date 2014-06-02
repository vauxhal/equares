var formatInfo = {}
formatInfo.init = function (elementId) {
    // Initialize MathJax
    MathJax.Hub.Config({
        elements: [elementId],
        TeX: {
            equationNumbers: { autoNumber: "AMS", useLabelIds: true},
        },
        tex2jax: {
            inlineMath: [['$','$']],
            processEscapes: true
        }
    })

    MathJax.Hub.Register.MessageHook("End Process", function (message) {
        var e = $('#' + elementId)
        e.html(marked(e.html()))
    })

    function LatexEquationNumbersReset() {
        var ams = MathJax.Extension["TeX/AMSmath"]
        ams.startNumber = 0
        ams.labels = {}
    }
    formatInfo.update = function(data) {
        data = data.replace("http:///", location.origin+'/')
        var rx = /(^| )(\?|=)\[([^\]]*)]\(([^)]*)\)/, rxg = new RegExp(rx.source, 'g')
        var m = data.match(rxg)
        if (m)
            for (var i=0; i<m.length; ++i) {
                var v = m[i], mm = data.match(rx)
                var a = mm[1] + '<a href="' + mm[4] + '"'
                if (mm[2] === '?')
                    a += ' target="_blank"'
                a += '>'
                data = data.replace(v, a + mm[3] + '</a>')
            }
        var e = $('#' + elementId)
        e.html(data)
        MathJax.Hub.Queue(LatexEquationNumbersReset, ["Typeset", MathJax.Hub, e[0]])
    }
    marked.setOptions({
        gfm: true
    });
}
