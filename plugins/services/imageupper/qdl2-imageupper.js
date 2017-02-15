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

var request = null;

// For galleries
var results = [];
var packageName = "";

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (url.indexOf("/g/") != -1) {
                // Gallery
                try {
                    var links = request.responseText.match(/http:\/\/imageupper\.com\/gi\/\?galID=\w+&n=\d+&m=\d+/g);

                    if ((links) && (links.length > 0)) {                        
                        for (var i = 0; i < links.length; i++) {
                            var link = links[i];
                            results.push(new UrlResult(link, link.split("&n=")[1].split("&")[0] + ".jpg"));
                        }

                        if (!packageName) {
                            var link = links[0];
                            packageName = link.split("galID=")[1].split("&")[0];
                        }

                        var next = /\/g\/\?galID=\w+&n=\d+(?=">>>)/.exec(request.responseText);
                        
                        if (next) {
                            checkUrl("http://imageupper.com" + next);
                        }
                        else {
                            urlChecked(results, packageName);
                            results = [];
                            packageName = "";
                        }
                    }
                    else if (results.length > 0) {
                        urlChecked(results, packageName);
                        results = [];
                        packageName = "";
                    }
                    else {
                        error(qsTr("File not found"));
                    }
                }
                catch(err) {
                    error(err);
                }
            }
            else {
                // Image
                try {
                    if (settings.value("retrieveGallery", false) == true) {
                        // Try to retrieve the gallery
                        var galleryLink = /\/g\/\?\w+/.exec(request.responseText);
                        
                        if (galleryLink) {
                            checkUrl("http://imageupper.com" + galleryLink);
                            return;
                        }
                    }
                    
                    var link = /http:\/\/cdn\.imageupper\.com\/[^"]+/.exec(request.responseText)[0];

                    if (link) {
                        urlChecked(new UrlResult(url, link.substring(link.lastIndexOf("/") + 1)));
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
    }

    request.open("GET", url);
    request.send();
}

function getDownloadRequest(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var link = /http:\/\/cdn\.imageupper\.com\/[^"]+/.exec(request.responseText)[0];

                if (link) {
                    downloadRequest(new NetworkRequest(link));
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
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
    }

    return true;
}
