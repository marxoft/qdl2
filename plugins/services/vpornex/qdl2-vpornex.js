/*!
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

var FORMATS = ["1080p", "720p", "480p", "360p"];
var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var fileName = /itemprop="headline">([^<]+)/.exec(request.responseText)[1] + ".mp4";

                try {
                    var docId = /docid=([^&"]+)/.exec(request.responseText)[1];
                    var gdUrl = "https://drive.google.com/file/d/" + docId + "/view";
                    urlChecked(new UrlResult(gdUrl, fileName));
                }
                catch(e) {
                    urlChecked(new UrlResult(url, fileName));
                }
            }
            catch(e) {
                error(e);
            }
        }
    }

    request.open("GET", url);
    request.send();
}

function getDownloadRequest(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var formats = JSON.parse(/sources:(\[[^\]]+\])/.exec(request.responseText)[1]);

                if (!formats.length) {
                    error(qsTr("No video formats found"));
                    return;
                }

                var options = [];

                for (var i = 0; i < FORMATS.length; i++) {
                    for (var j = 0; j < formats.length; j++) {
                        if (formats[j].label == FORMATS[i]) {
                            options.push({"label": formats[j].label.toUpperCase(), "value": formats[j].file});
                            break;
                        }
                    }
                }

                if (settings.value("useDefaultFormat", false) === true) {
                    var format = settings.value("format", "1080P");

                    for (var i = 0; i < options.length; i++) {
                        if (options[i].label == format) {
                            downloadRequest(new NetworkRequest(options[i].value));
                            return;
                        }
                    }

                    downloadRequest(new NetworkRequest(options[options.length - 1].value));
                }
                else {
                    var list = {"type": "list", "label": qsTr("Video format"), "key": "url", "options": options,
                                "value": options[0].value};
                    settingsRequest(qsTr("Choose video format"), [list],
                        function(f) { downloadRequest(new NetworkRequest(f.url)); });
                }
            }
            catch(e) {
                error(e);
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

