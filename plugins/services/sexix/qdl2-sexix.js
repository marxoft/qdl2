/**
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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
    var PLAYLIST_URL = "http://sexix.net/qaqqew/playlist.php?u=";
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        var request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var title = /<title>([^<]+)/.exec(request.responseText)[1];
                    title = title.substring(0, title.lastIndexOf(" - Watch"));
                    plugin.urlChecked(new UrlResult(url, title + ".mp4"));
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
        var request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var id = /sexix\.net\/v\.php\?u=([^"]+)/.exec(request.responseText)[1];
                    request = new XMLHttpRequest();
                    request.onreadystatechange = function() {
                        if (request.readyState == 4) {
                            var response = request.responseText;
                            var formats = [];
                            var re = /file="([^"]+).+label="([^"]+)/g;
                            var res = null;
                            
                            while (res = re.exec(response)) {
                                formats.push({"label": res[2].toUpperCase(), "value": res[1]});
                                re.lastIndex = res.index + 1;
                            }
                            
                            if (!formats.length) {
                                plugin.error(qsTr("No video formats found"));
                                return;
                            }
                            
                            if (settings.useDefaultVideoFormat) {
                                var format = settings.videoFormat || "1080P";
                                
                                for (var i = 0; i < formats.length; i++) {
                                    if (formats[i].label == format) {
                                        plugin.downloadRequest(new NetworkRequest(formats[i].value));
                                        return;
                                    }
                                }
                                
                                plugin.downloadRequest(new NetworkRequest(formats[0].value));
                            }
                            else {
                                var list = {"type": "list", "label": qsTr("Format"), "key": "url",
                                            "value": formats[0].value, "options": formats};
                                plugin.settingsRequest(qsTr("Choose video format"), [list], function(f) {
                                    plugin.downloadRequest(new NetworkRequest(f.url));
                                });
                            }
                        }
                    }
                    
                    request.open("GET", PLAYLIST_URL + id);
                    request.send();
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
