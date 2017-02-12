/*
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

var USER_AGENT = "Wget/1.13.4 (linux-gnu)";
var STREAMS_COOKIE = "s=3lu41hy4i1seitbhbtqkt5x5v94wwziy";

var request = null;

function checkUrl(url) {
    var apiUrl = url.replace("www.mixcloud.com", "api.mixcloud.com");
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState === 4) {
            try {
                var response = JSON.parse(request.responseText);
                urlChecked({"fileName": response.name + ".m4a", "url": url});
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", apiUrl);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function getDownloadRequest(url) {
    url = url.replace("api.mixcloud.com", "www.mixcloud.com");
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState === 4) {
            try {
                var response = request.responseText;
                var previewUrl = /m-preview="([^"]+)/.exec(response)[1];
                var streamHost = /stream(\d+)\.mixcloud\.com/.exec(response)[0];
                var streamUrl = "http://" + streamHost + previewUrl.split("mixcloud.com")[1]
                    .replace("/previews/", "/c/m4a/64/").replace(/\.mp3$/, ".m4a");
                downloadRequest({"url": streamUrl});
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.setRequestHeader("Cookie", STREAMS_COOKIE);
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}
