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
        if (url.indexOf("/get_file/") != -1) {
            var fileName = /\d+_\w+\.mp4/.exec(url);
            
            if (!fileName) {
                fileName = url.substring(url.lastIndexOf("/") + 1);
            }
            
            plugin.urlChecked(new UrlResult(url, fileName));
            return;
        }
        
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var videoString = request.responseText.split("window.video = ")[1].split(";\n")[0];
                    videoString = videoString.replace(/'/g, "\"");
                    var re = /([a-zA-Z]+)(: )/;
                    var i = 0;
                    
                    while ((re.test(videoString)) && (i++ < 50)) {
                        videoString = videoString.replace(re, "\"$1\"$2");
                    }
                                    
                    var playlist = JSON.parse(videoString).playlist;
                    var results = [];
                    
                    for (var i = 0; i < playlist.length; i++) {
                        var title = playlist[i].title;
                        var source = playlist[i].sources[0];
                        results.push(new UrlResult(source.file, title + "." + source.type));
                    }
                    
                    if (results.length > 0) {
                        plugin.urlChecked(results, playlist[0].title);
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

    plugin.getDownloadRequest = function(url) {
        if (url.indexOf("/get_file/") != -1) {
            plugin.downloadRequest(new NetworkRequest(url));
            return;
        }
        
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var videoString = request.responseText.split("window.video = ")[1].split(";\n")[0];
                    videoString = videoString.replace(/'/g, "\"");
                    var re = /([a-zA-Z]+)(: )/;
                    var i = 0;
                    
                    while ((re.test(videoString)) && (i++ < 50)) {
                        videoString = videoString.replace(re, "\"$1\"$2");
                    }
                    
                    var video = JSON.parse(videoString).playlist[0];
                    var videoUrl = video.sources[0].file;

                    if (videoUrl) {
                        plugin.downloadRequest(new NetworkRequest(videoUrl));
                    }
                    else {
                        plugin.error(qsTr("Unknown error"));
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
