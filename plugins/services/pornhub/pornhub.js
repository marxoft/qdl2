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

var VIDEO_FORMATS = ["1080", "720", "480", "240"];

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var fileName = request.responseText.split("twitter:title\" content=\"")[1].split("\"")[0];
                
                if (fileName) {
                    urlChecked({"url": url, "fileName": fileName + ".mp4"});
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
    request.setRequestHeader("Cookie", "platform=mobile");
    request.send();
}

function getDownloadRequest(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var page = request.responseText;
                var format = settings.value("videoFormat", "1080");

                if (settings.value("useDefaultVideoFormat", true) == true) {
                    for (var i = Math.max(0, VIDEO_FORMATS.indexOf(format)); i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = page.split("var player_quality_" + VIDEO_FORMATS[i] + "p = '")[1].split("'")[0];
                            
                            if (videoUrl) {
                                if (videoUrl.substring(0, 1) == "/") {
                                    videoUrl = "http:" + videoUrl;
                                }
                                
                                downloadRequest({"url": videoUrl});
                                return;
                            }
                        }
                        catch(err) {
                            continue;
                        }
                    }
                }
                else {
                    var options = [];

                    for (var i = 0; i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = page.split("var player_quality_" + VIDEO_FORMATS[i] + "p = '")[1].split("'")[0];
                            
                            if (videoUrl) {
                                if (videoUrl.substring(0, 1) == "/") {
                                    videoUrl = "http:" + videoUrl;
                                }
                                
                                options.push({"label": VIDEO_FORMATS[i] + "P", "value": videoUrl});
                            }
                        }
                        catch(err) {
                            continue;
                        }
                    }

                    if (options.length > 0) {
                        settingsRequest(qsTr("Video format"), [{"type": "list", "label": qsTr("Video format"),
                                                          "key": "url",
                                                          "value": options[Math.max(0, options.indexOf(format))].value,
                                                          "options": options}],
                                        function (f) { downloadRequest({"url": f.url}); });
                        return;
                    }
                }

                error(qsTr("Unknown error"));
            }
            catch(err) {
                error(err);
            }
        }
    }

    request.open("GET", url);
    request.setRequestHeader("Cookie", "platform=mobile");
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
    }

    return true;
}
