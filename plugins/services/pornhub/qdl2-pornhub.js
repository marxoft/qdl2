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

var MOBILE_COOKIE = "platform=mobile";

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var fileName = request.responseText.split("twitter:title\" content=\"")[1].split("\"")[0];
                
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
    request.setRequestHeader("Cookie", MOBILE_COOKIE);
    request.send();
}

function getDownloadRequest(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var script = /var varObj =[^<]+/.exec(request.responseText)[0].trim();
                var qitems = /var (qualityItems_\d+)/.exec(script)[1];
                eval(script);
                var formats = eval(qitems);
                
                if (!formats.length) {
                    error(qsTr("No video formats found"));
                    return;
                }
                
                if (settings.value("useDefaultVideoFormat", true) == true) {
                    var format = settings.value("videoFormat", "quality1080p");
                    
                    for (var i = formats.length - 1; i > 0; i--) {
                        if (formats[i].id == format) {
                            downloadRequest(new NetworkRequest(formats[i].url));
                            return;
                        }
                    }
                    
                    downloadRequest({"url": formats[0].url});
                }
                else {
                    var list = {"type": "list", "label": qsTr("Video format"), "key": "format"};
                    var options = [];
                    
                    for (var i = formats.length - 1; i >= 0; i--) {
                        options.push({"label": formats[i].text.toUpperCase(), "value": formats[i].url});
                    }
                    
                    list["options"] = options;
                    list["value"] = formats[formats.length - 1].url;
                    settingsRequest(qsTr("Choose video format"), [list], function(s) {
                        downloadRequest(new NetworkRequest(s.format));
                    });
                }
            }
            catch(err) {
                error(err);
            }
        }
    }

    request.open("GET", url);
    request.setRequestHeader("Cookie", MOBILE_COOKIE);
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}
