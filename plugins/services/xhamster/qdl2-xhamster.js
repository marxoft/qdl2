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
var EMBED_URL = "https://xhamster.com/xembed.php?video=";

var request = null;

function checkUrl(url) {
    var id = url.substr(url.lastIndexOf("/") + 1);
    id = id.substr(id.lastIndexOf("-") + 1);

    if (!id) {
        error(qsTr("No video ID found in URL"));
        return;
    }

    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var vars = JSON.parse(request.responseText.split("flashVars = ")[1].split(";\n")[0]);
                var fileName = vars.title;
                
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

    request.open("GET", EMBED_URL + id);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function getDownloadRequest(url) {
    var id = url.substr(url.lastIndexOf("/") + 1);
    id = id.substr(id.lastIndexOf("-") + 1);

    if (!id) {
        error(qsTr("No video ID found in URL"));
        return;
    }

    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var sources = JSON.parse(/sources: (\{[^\}]+\})/.exec(request.responseText)[1]);
                var format = settings.value("videoFormat", "1080p");

                if (settings.value("useDefaultVideoFormat", true) == true) {
                    for (var i = Math.max(0, VIDEO_FORMATS.indexOf(format)); i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = sources[VIDEO_FORMATS[i]];
                            
                            if (videoUrl) {
                                downloadRequest(new NetworkRequest(videoUrl));
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
                            var videoUrl = sources[VIDEO_FORMATS[i]];
                            
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
                                        function (f) { downloadRequest(new NetworkRequest(f.url)); });
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

    request.open("GET", EMBED_URL + id);
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
