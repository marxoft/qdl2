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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var USER_AGENT = "Wget/1.13.4 (linux-gnu)";
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        var fragment = url.substring(url.lastIndexOf("/") + 1);
        var fileName = fragment.substring(0, fragment.lastIndexOf(".html"));
        
        if (fileName) {
            plugin.urlChecked(new UrlResult(url, fileName));
        }
        else {
            plugin.error(qsTr("Filename not found"));
        }
    };

    plugin.getDownloadRequest = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {            
                try {
                    var sources = JSON.parse(/sources: (\[.+\])/.exec(request.responseText)[1]);
                    
                    while (sources.length > 0) {
                        var videoUrl = sources.pop();
                        
                        if (videoUrl.substring(videoUrl.length - 4) == ".mp4") {
                            plugin.downloadRequest(new NetworkRequest(videoUrl));
                            return;
                        }
                    }
                    
                    plugin.error(qsTr("No video stream found"));
                }
                catch(err) {
                    plugin.error(err);
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
