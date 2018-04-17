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
    var FILE_NAME_REGEXP = /"og:title" content="([^"]+)/;
    var VIDEO_FORMATS = ["video_alt_url3", "video_alt_url2", "video_alt_url", "video_url"];
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var fileName = FILE_NAME_REGEXP.exec(request.responseText)[1];
                    
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
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var formats = [];
                    
                    for (var i = 0; i < VIDEO_FORMATS.length; i++) {
                        var re = new RegExp(VIDEO_FORMATS[i] + ": '([^']+).+" + VIDEO_FORMATS[i] + "_text: '([^']+)");
                        var f = re.exec(response);
                        
                        if (f) {
                            formats.push({"label": f[2].toUpperCase(), "value": f[1]});
                        }
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
                        var list = {"type": "list", "label": qsTr("Video format"), "key": "url",
                            "value": formats[0].value, "options": formats};
                        plugin.settingsRequest(qsTr("Choose video format"), [list], function(f) {
                            plugin.downloadRequest(new NetworkRequest(f.url));
                        });
                    }
                }
                catch(err) {
                    plugin.error(err);
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
