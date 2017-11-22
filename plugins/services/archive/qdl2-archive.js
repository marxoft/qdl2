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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var DOWNLOAD_URL = "https://archive.org/download/";
    var request = null;

    function checkFile(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                switch (request.status) {
                    case 200:
                    case 302:
                        plugin.urlChecked(new UrlResult(url, url.substring(url.lastIndexOf("/") + 1)));
                        break;
                    default:
                        plugin.error(qsTr("File not found"));
                        break;
                }
            }
        }

        request.followRedirects = false;
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
                    var options = [];

                    for (var i = 0; i < files.length; i++) {
                        options.push({"label": files[i].name, "value": files[i].name});
                    }

                    var list = {"type": "list", "multiselect": true, "label": qsTr("Files"), "key": "files",
                                "options": options};
                    plugin.settingsRequest(qsTr("Choose files"), [list], function(params) {
                        var files = params.files;

                        if (!files.length) {
                            error(qsTr("No files chosen"));
                            return;
                        }

                        var results = [];
                        
                        for (var i = 0; i < files.length; i++) {
                            results.push(new UrlResult(DOWNLOAD_URL + packageName + "/" + files[i], files[i]));
                        }

                        plugin.urlChecked(results, packageName);
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

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        if (url.indexOf("archive.org/download/") != -1) {
            if (url.match(/\//g).length > 4) {
                checkFile(url);
                return;
            }
        }

        url = url.replace(/archive.org\/(details|download)/, "archive.org/metadata/");
        checkFiles(url);
    };

    plugin.getDownloadRequest = function(url) {
        plugin.downloadRequest(new NetworkRequest(url));
    };

    plugin.cancelCurrenOperation = function() {
        if (request) {
            request.abort();
            request = null;
        }

        return true;
    };

    return plugin;
})
