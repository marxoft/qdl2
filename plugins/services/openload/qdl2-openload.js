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

var VIDEO_URL = "https://openload.co/embed/";
var STREAM_URL = "https://openload.co/stream/";

var USER_AGENT = "Wget/1.13.4 (linux-gnu)";

var request = null;

function stringToInt(s) {
    var i = 0;
    
    while (i < s.length - 1) {
        if (s[i] !== "0") {
            break;
        }
        
        i++;
    }
    
    return parseInt(s.substring(i));
}

function checkUrl(url) {
    try {
        var id = /openload.co\/(f|embed)\/([\w-_]+)/.exec(url)[2];
        url = VIDEO_URL + id;
    }
    catch(err) {
        error(err);
        return;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;
            
            try {
                var fileName = /<meta name="og:title" content="([^"]+)">/.exec(response)[1];
                urlChecked(new UrlResult(url, fileName));
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function getDownloadRequest(url) {
    try {
        var id = /openload.co\/(f|embed)\/([\w-_]+)/.exec(url)[2];
        url = VIDEO_URL + id;
    }
    catch(err) {
        error(err);
        return;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;
            
            try {
                var openloadId = /<span id="[^"]+">(\d+)<\/span>/.exec(response)[1];
                var three = stringToInt(openloadId.substr(0, 3));
                var fifth = stringToInt(openloadId.substr(3, 2));
                var urlcode = "";
                var num = 5;
                
                while (num < openloadId.length) {
                    urlcode += String.fromCharCode(stringToInt(openloadId.substr(num, 3)) + three - fifth
                                                   * stringToInt(openloadId.substr(num + 3, 2)));
                    num += 5;
                }
                
                var videoUrl = STREAM_URL + urlcode;
                downloadRequest(new NetworkRequest(videoUrl));
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }
    
    return true;
}
