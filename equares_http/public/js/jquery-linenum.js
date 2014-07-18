/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

// See also:
// jQuery Lined Textarea Plugin by Alan Williamson
// http://alan.blog-city.com/jquerylinedtextarea.htm
// We have a different solution here...

(function($) {
    $.fn.linenum = function() {
        return this.each(function() {
            var t0 = this, t = $(t0).addClass("linenum-text")
            t.wrap('<div class="linenum-text"></div>').attr("wrap", "off")
            var wt = t.parent(),
                wln = $('<div class="linenum-count"></div>').insertBefore(wt),
                ln = $('<textarea class="linenum-count"></textarea>')
                    .prop("disabled", true)
                    .appendTo(wln),
                ln0 = ln[0]
            var n = 0

            // Have to set line height explicitly
            t.add(ln).css("line-height", "16px")

            // http://makandracards.com/makandra/8247-scroll-a-textarea-to-a-given-line-with-jquery
            // This assumes your textarea has a line-height in Pixels. If that is not the case, you must do crazy things.
            var lh = parseInt(t.css('line-height'))

            function scrollLineNum() {
                var h = t0.scrollHeight + 10
                var nmin = ln.height() / lh
                while (ln0.scrollHeight < h || n < nmin)
                    ln0.value += ++n + "\n"
                ln0.scrollTop = t0.scrollTop
            }
            scrollLineNum()
            t.scroll(scrollLineNum).resize(scrollLineNum)
        })
    }
})(jQuery);

