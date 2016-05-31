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
var page = 1;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (url.indexOf("/gallery/") != -1) {
                // Gallery
                try {
                    var gallery = request.responseText.split("<fieldset>")[1].split("</fieldset>")[0];
                    var links = gallery.match(/http(s|):\/\/www\.imagebam\.com\/image\/\w+/g);

                    if ((links) && (links.length > 0)) {                        
                        for (var i = 0; i < links.length; i++) {
                            var link = links[i];
                            results.push({"url": link, "fileName": link.substring(link.lastIndexOf("/") + 1) + ".jpg"});
                        }

                        if (!packageName) {
                            try {
                                packageName = /[^>]+(?=<span)/.exec(gallery);
                            }
                            catch(err) {
                                var link = links[0];
                                packageName = link.substring(link.lastIndexOf("/") + 1);
                            }
                        }

                        if (gallery.indexOf("pagination_link") != -1) {
                            if (page > 1) {
                                checkUrl(url.substring(0, url.lastIndexOf("/") + 1) + ++page);
                            }
                            else {
                                checkUrl(url + "/" + ++page);
                            }
                        }
                        else {
                            urlChecked(results, packageName);
                            results = [];
                            packageName = "";
                            page = 1;
                        }
                    }
                    else if (results.length > 0) {
                        urlChecked(results, packageName);
                        results = [];
                        packageName = "";
                        page = 1;
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
                        var galleryLink = /\/gallery\/\w+/.exec(request.responseText);
                        
                        if (galleryLink) {
                            checkUrl("http://imagebam.com" + galleryLink);
                            return;
                        }
                    }
                        
                    var link = decodeURIComponent(request.responseText.split("og:image\" content=\"")[1].split("\"")[0]);

                    if (link) {
                        urlChecked({"url": url, "fileName": link.substring(link.lastIndexOf("/") + 1)});
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
                var link = decodeURIComponent(request.responseText.split("og:image\" content=\"")[1].split("\"")[0]);

                if (link) {
                    downloadRequest({"url": link});
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
