/**
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var ALPHABET = "=/+9876543210zyxwvutsrqponmlkjihgfedcbaZYXWVUTSRQPONMLKJIHGFEDCBA";
    var request = null;
    var plugin = new ServicePlugin();

    function decryptSrc(encoded, val) {
        encoded = encoded.replace(/[^a-zA-Z0-9\+\/=]/g, "");
        var decoded = "";
        var sm = [null, null, null, null];
        var i = 0;
        var len = encoded.length;

        while (i < len) {
            for (var j = 0; j < 4; j++) {
                sm[j % 4] = ALPHABET.indexOf(encoded[i]);
                ++i;
            }

            var charCode = ((sm[0] << 0x2) | (sm[1] >> 0x4)) ^ val;
            decoded += String.fromCharCode(charCode);

            if (sm[2] != 0x40) {
                charCode = ((sm[1] & 0xf) << 0x4) | (sm[2] >> 0x2);
                decoded += String.fromCharCode(charCode);
            }

            if (sm[3] != 0x40) {
                charCode = ((sm[2] & 0x3) << 0x6) | sm[3];
                decoded += String.fromCharCode(charCode);
            }
        }

        if (decoded[0] == "/") {
            decoded = "https:" + decoded;
        }

        return decoded;
    }

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var fileName = decodeHtml(/"og:title" content="([^"]+)/.exec(request.responseText)[1]);
                    plugin.urlChecked(new UrlResult(url, fileName));
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var formats = [];
                    var re = /type:"video\/mp4",src:d\('([^']+)',(\d+)\),height:(\d+)/g;
                    var match;

                    while (match = re.exec(response)) {
                        try {
                            var encoded = match[1];
                            var val = match[2];
                            var height = match[3];
                            var src = decryptSrc(encoded, val);

                            if (src) {
                                formats.unshift({"label": height + "P", "value": src});
                            }

                            re.lastIndex = match.index + match[0].length;
                        }
                        catch(e) {
                            re.lastIndex = match.index + match[0].length;
                        }
                    }

                    if (!formats.length) {
                        plugin.error(qsTr("No video formats found"));
                        return;
                    }
                    
                    var format = settings.videoFormat || "1080P";

                    if (settings.useDefaultVideoFormat) {
                        for (var i = 0; i < formats.length; i++) {
                            if (formats[i].label == format) {
                                plugin.downloadRequest(new NetworkRequest(formats[i].value));
                                return;
                            }
                        }

                        plugin.downloadRequest(new NetworkRequest(formats[0].value));
                    }
                    else {
                        plugin.settingsRequest(qsTr("Video format"), [{"type": "list", "label": qsTr("Video format"),
                            "key": "url", "value": formats[Math.max(0, formats.indexOf(format))].value,
                            "options": formats}], function (f) { plugin.downloadRequest(new NetworkRequest(f.url)); });
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.cancelCurrentOperation = function() {
        if (request) {
            request.abort();
            request = null;
        }

        return true;
    };

    return plugin;
})
