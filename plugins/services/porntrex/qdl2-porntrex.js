/*
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

var FILE_NAME_REGEXP = /<title>(.+)<\/title>/
var VIDEO_FORMATS = ["video_alt_url3", "video_alt_url2", "video_alt_url", "video_url"];
var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var fileName = FILE_NAME_REGEXP.exec(request.responseText)[1];
                
                if (fileName) {
                    urlChecked(new UrlResult(url, fileName + ".mp4"));
                }
                else {
                    error(qsTr("File not found"));
                }
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    
    request.open("GET", url);
    request.send();
}

function getDownloadRequest(url) {
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
                    error(qsTr("No video formats found"));
                    return;
                }
                
                if (settings.value("useDefaultVideoFormat", true) == true) {
                    var format = settings.value("videoFormat", "1080P");
                    
                    for (var i = 0; i < formats.length; i++) {
                        if (formats[i].label == format) {
                            downloadRequest(new NetworkRequest(formats[i].value));
                            return;
                        }
                    }
                    
                    downloadRequest(new NetworkRequest(formats[0].value));
                }
                else {
                    var list = {"type": "list", "label": qsTr("Video format"), "key": "url", "value": formats[0].value,
                               "options": formats};
                    settingsRequest(qsTr("Choose video format"), [list], function(f) {
                        downloadRequest(new NetworkRequest(f.url));
                    });
                }
            }
            catch(err) {
                error(err);
            }
        }
    }

    request.open("GET", url);
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}
