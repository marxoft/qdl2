/**
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

                try {
                    var sources = JSON.parse(/"sources": (\[[^\]]+\])/.exec(response)[1]);

                    if (!sources.length) {
                        error(qsTr("No video formats found"));
                        return;
                    }

                    if (settings.value("useDefaultFormat", false) === true) {
                        var format = settings.value("format", "1080");

                        for (var i = sources.length - 1; i >= 0; i--) {
                            var source = sources[i];

                            if (source.res == format) {
                                downloadRequest(new NetworkRequest(source.file));
                                return;
                            }
                        }

                        downloadRequest(new NetworkRequest(sources[0].file));
                        return;
                    }

                    var options = [];

                    for (var i = sources.length - 1; i >= 0; i--) {
                        var source = sources[i];
                        options.push({"label": source.res + "P", "value": source.file});
                    }

                    settingsRequest(qsTr("Choose video format"), [{"type": "list", "key": "url",
                        "label": qsTr("Video format"), "options": options, "value": options[0].value}],
                        function (conf) { downloadRequest(new NetworkRequest(conf.url)); });
                }
                catch(e) {
                    var source = /<source src="([^"]+)/.exec(request.responseText)[1];
                    downloadRequest(new NetworkRequest(source));
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

