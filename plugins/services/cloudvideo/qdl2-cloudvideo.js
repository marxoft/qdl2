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
    var USER_AGENT = "Wget/1.13.4 (linux-gnu)";
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        url = url.replace("/embed-", "/");
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {            
                try {
                    var fileName = decodeHtml(/data-title="([^"]+)/.exec(request.responseText)[1]) + ".mp4";
                    plugin.urlChecked(new UrlResult(url, fileName));
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }
        
        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {            
                try {
                    var json = /sources: (\[.+\])/.exec(request.responseText)[1];
                    json = json.replace(/file:/g, "\"value\":").replace(/label:/g, "\"label\":");
                    var formats = JSON.parse(json);

                    if (formats.length < 2) {
                        plugin.error(qsTr("No video formats found"));
                        return;
                    }

                    formats.shift();

                    if (settings.useDefaultFormat) {
                        var format = settings.format || "720p";

                        for (var i = 0; i < formats.length; i++) {
                            if (formats[i].label == format) {
                                plugin.downloadRequest(new NetworkRequest(formats[i].value));
                                return;
                            }
                        }

                        plugin.downloadRequest(new NetworkRequest(formats[0].value));
                    }
                    else {
                        var list = {"type": "list", "label": qsTr("Video format"), "key": "format", "options": formats,
                            "value": formats[0].value};
                        plugin.settingsRequest(qsTr("Choose video format"), [list], function(result) {
                            plugin.downloadRequest(new NetworkRequest(result.format));
                        });
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }
        
        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
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
