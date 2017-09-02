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

var FORMATS = ["720p", "480p", "360p"];
var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var fileName = /"og:title" content="([^"]+)/.exec(request.responseText)[1];
                urlChecked(new UrlResult(url, fileName));
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
                var response = request.responseText;
                var options = [];

                for (var i = 0; i < FORMATS.length; i++) {
                    try {
                        var re =
                            new RegExp("<source src=\"([^\"]+)\" type=\"video/mp4\" title=\"" + FORMATS[i] + "\"");
                        options.push({"label": FORMATS[i].toUpperCase(), "value": re.exec(response)[1]});
                    }
                    catch(e) {
                        continue;
                    }
                }

                if (!options.length) {
                    error(qsTr("No video formats found"));
                    return;
                }

                if (settings.value("useDefaultFormat", false) === true) {
                    var format = settings.value("format", "720P");

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

    request.open("GET", url + "&q=0p"); // Ensure full list of formats appears
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}

