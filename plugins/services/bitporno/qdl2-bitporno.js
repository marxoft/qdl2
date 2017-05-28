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

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var fileName = /<title>([^<]+)<\/title>/.exec(request.responseText)[1];
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
                var formats = JSON.parse(/"sources": (\[[^\]]+\])/.exec(request.responseText)[1]);

                if (formats.length < 2) {
                    error(qsTr("No video formats found"));
                    return;
                }

                formats.pop();

                if (settings.value("useDefaultFormat", false) === true) {
                    var format = settings.value("format", "1080P HD");

                    for (var i = 0; i < formats.length; i++) {
                        if (formats[i].label.toUpprCase() == format) {
                            downloadRequest(new NetworkRequest(formats[i].file));
                            return;
                        }
                    }

                    downloadRequest(new NetworkRequest(formats[formats.length - 1].file));
                }
                else {
                    var options = [];

                    for (var i = formats.length - 1; i >= 0; i--) {
                        options.push({"label": formats[i].label.toUpperCase(), "value": formats[i].file});
                    }

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

