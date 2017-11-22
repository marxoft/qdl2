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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var COOKIE = "platform=mobile";
    var USER_AGENT = "Mozilla/5.0 (X11; Linux x86_64; rv:51.0) Gecko/20100101 Firefox/51.0";
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var fileName = request.responseText.split("twitter:title\" content=\"")[1].split("\"")[0];
                    
                    if (fileName) {
                        plugin.urlChecked(new UrlResult(url, fileName + ".mp4"));
                    }
                    else {
                        plugin.error(qsTr("File not found"));
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("Cookie", COOKIE);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var formats = JSON.parse(/var qualityItems_\d+\s*=\s*(\[[^\[]+\])/.exec(request.responseText)[1]);

                    if (!formats.length) {
                        plugin.error(qsTr("No video formats found"));
                        return;
                    }
                    
                    if (settings.useDefaultVideoFormat) {
                        var format = settings.videoFormat || "quality1080p";
                        
                        for (var i = formats.length - 1; i > 0; i--) {
                            if (formats[i].id == format) {
                                plugin.downloadRequest(new NetworkRequest(formats[i].url));
                                return;
                            }
                        }
                        
                        plugin.downloadRequest(new NetworkRequest(formats[formats.length - 1].url));
                    }
                    else {
                        var list = {"type": "list", "label": qsTr("Video format"), "key": "format"};
                        var options = [];
                        
                        for (var i = formats.length - 1; i >= 0; i--) {
                            options.push({"label": formats[i].text.toUpperCase(), "value": formats[i].url});
                        }
                        
                        list["options"] = options;
                        list["value"] = formats[formats.length - 1].url;
                        plugin.settingsRequest(qsTr("Choose video format"), [list], function(s) {
                            plugin.downloadRequest(new NetworkRequest(s.format));
                        });
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("Cookie", COOKIE);
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
