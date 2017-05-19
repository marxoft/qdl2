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

var DOWNLOAD_URL = "https://archive.org/download/";
var request = null;

function checkUrl(url) {
    if (url.indexOf("archive.org/download/") != -1) {
        if (url.match(/\//g).length > 4) {
            checkFile(url);
            return;
        }
    }

    url = url.replace(/archive.org\/(details|download)/, "archive.org/metadata/");
    checkFiles(url);
}

function checkFile(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            switch (request.status) {
                case 200:
                case 302:
                    urlChecked(new UrlResult(url, url.substring(url.lastIndexOf("/") + 1)));
                    break;
                default:
                    error(qsTr("File not found"));
                    break;
            }
        }
    }

    request.open("HEAD", url);
    request.send();
}

function checkFiles(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var files = JSON.parse(request.responseText).files;

                if (!files.length) {
                    error(qsTr("No files found"));
                    return;
                }

                var packageName = url.substring(url.lastIndexOf("/") + 1);
                var options = [{"label": qsTr("All files"), "value": ""}];

                for (var i = 0; i < files.length; i++) {
                    options.push({"label": files[i].name, "value": files[i].name});
                }

                var list = {"type": "list", "label": qsTr("File"), "key": "file", "options": options};
                settingsRequest(qsTr("Choose file"), [list], function(params) {
                    var results = [];

                    if (params.file) {
                        urlChecked(new UrlResult(DOWNLOAD_URL + packageName + "/" + params.file, params.file));
                    }
                    else {
                        var results = [];

                        for (var i = 1; i < options.length; i++) {
                            results.push(new UrlResult(DOWNLOAD_URL + packageName + "/" + options[i].value,
                                options[i].value));
                        }

                        urlChecked(results, packageName);
                    }
                });
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
    downloadRequest(new NetworkRequest(url));
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}

