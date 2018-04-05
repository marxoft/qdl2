/**
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        url = url.replace("m.spankbang.com", "spankbang.com");
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var fileName = decodeHtml(/h1 title="([^"]+)/.exec(request.responseText)[1]) + ".mp4";
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
        url = url.replace("m.spankbang.com", "spankbang.com");
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var formats = [];
                    var re = /stream_url_(\d+(p|k))\s+=\s+'([^']+)/g;
                    var match;

                    while (match = re.exec(response)) {
                        try {
                            formats.unshift({"label": match[1].toUpperCase(), "value": match[3]});
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
                    
                    var format = settings.videoFormat || "4K";

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
