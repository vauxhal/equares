/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var MobileDetect = require('mobile-detect');

function Cache(maxSize) {
    this.data = {}
    this.size = 0
    this.maxSize = maxSize || 10000
}

Cache.prototype.value = function(key) {
    return this.data[key]
}

Cache.prototype.setValue = function(key, value) {
    this.maybeClear()
    this.data[key] = value
    ++this.size
}

Cache.prototype.clear = function() {
    this.data = {}
    this.size = 0
}

Cache.prototype.maybeClear = function() {
    if (this.size > this.maxSize)
        this.clear()
}

var cache = new Cache

module.exports = function(req) {
    var ua = req.headers['user-agent']
    var v = cache.value(ua)
    if (v === undefined) {
        var md = new MobileDetect(ua)
        v = md.mobile() ?   true :   false
        cache.setValue(ua, v)
    }
    return v
}
