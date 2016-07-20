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

var VIDEO_FORMATS = ["1080p", "720p", "480p", "240p"];
var USER_AGENT = "Wget/1.13.4 (linux-gnu)";

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var vars = JSON.parse(request.responseText.split("vars: ")[1].split(",\n")[0]);
                var fileName = vars["new"].title;
                
                if (fileName) {
                    urlChecked({"url": url, "fileName": fileName + ".flv"});
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
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function getDownloadRequest(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var vars = JSON.parse(request.responseText.split("vars: ")[1].split(",\n")[0]);
                var videoUrls = JSON.parse(vars["new"].videoUrls);
                var format = settings.value("videoFormat", "1080p");

                if (settings.value("useDefaultVideoFormat", true) == true) {
                    for (var i = Math.max(0, VIDEO_FORMATS.indexOf(format)); i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = videoUrls[VIDEO_FORMATS[i]][0];
                            
                            if (videoUrl) {
                                downloadRequest({"url": videoUrl});
                                return;
                            }
                        }
                        catch(err) {
                            continue;
                        }
                    }

                    error(qsTr("Unknown error"));
                }
                else {
                    var options = [];
                    
                    for (var i = 0; i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = videoUrls[VIDEO_FORMATS[i]][0];
                            
                            if (videoUrl) {
                                options.push({"label": VIDEO_FORMATS[i].toUpperCase(), "value": videoUrl});
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
                    }
                    else {
                        error(qsTr("Unknown error"));
                    }
                }
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
    }

    return true;
}
