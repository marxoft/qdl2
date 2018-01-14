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
    var VIDEO_FORMATS = ["1080p", "720p", "480p"];
    var request = null;

    function getVideoUrl(streamId, streamKey, format) {
        return "http://spankbang.com/_" + streamId + "/" + streamKey + "/title/" + format + "__mp4";
    }

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        url = url.replace("m.spankbang.com", "spankbang.com");
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var streamKey = request.responseText.split("stream_key  = '")[1].split("'")[0];
                    var streamId = request.responseText.split("stream_id  = '")[1].split("'")[0];
                    var fileName = request.responseText.split("h1 title=\"")[1].split("\"")[0];
                    
                    if ((streamKey) && (streamId) && (fileName)) {
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
        url = url.replace("m.spankbang.com", "spankbang.com");
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var streamKey = request.responseText.split("stream_key  = '")[1].split("'")[0];
                    var streamId = request.responseText.split("stream_id  = '")[1].split("'")[0];
                    
                    if ((!streamKey) || (!streamId)) {
                        plugin.error(qsTr("File not found"));
                        return;
                    }
                    
                    var shd = parseInt(request.responseText.split("stream_shd  = ")[1].split(";")[0]);
                    var hd = parseInt(request.responseText.split("stream_hd  = ")[1].split(";")[0]);
                    var formats = VIDEO_FORMATS;
                    
                    if (shd != 1) {
                        formats.splice(0, 1);
                    }
                    
                    if (hd != 1) {
                        formats.splice(0, 1);
                    }
                    
                    var format = settings.videoFormat || "1080p";

                    if (settings.useDefaultVideoFormat) {
                        var videoUrl = getVideoUrl(streamId, streamKey, formats[Math.max(0, formats.indexOf(format))]);
                        plugin.downloadRequest(new NetworkRequest(videoUrl));
                    }
                    else {
                        var options = [];
                        
                        for (var i = 0; i < formats.length; i++) {
                            var videoUrl = getVideoUrl(streamId, streamKey, formats[i]);
                            options.push({"label": formats[i].toUpperCase(), "value": videoUrl});
                        }

                        plugin.settingsRequest(qsTr("Video format"), [{"type": "list", "label": qsTr("Video format"),
                            "key": "url", "value": options[Math.max(0, options.indexOf(format))].value,
                            "options": options}], function (f) { plugin.downloadRequest(new NetworkRequest(f.url)); });
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
