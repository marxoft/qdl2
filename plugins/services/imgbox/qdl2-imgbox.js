/**
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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                if (url.indexOf("/g/") != -1) {
                    // Gallery
                    try {
                        var gallery = request.responseText.split("gallery-view-content")[1].split("<div")[0];
                        var ids = gallery.match(/\w+(?=\"><img)/g);

                        if ((ids) && (ids.length > 0)) {
                            var results = [];
                            
                            for (var i = 0; i < ids.length; i++) {
                                results.push(new UrlResult("http://imgbox.com/" + ids[i], ids[i]));
                            }

                            try {
                                var packageName = request.responseText.split("id=\"gallery-view\"")[1]
                                    .split("<div")[0].split("<h1>")[1].split("</h1>")[0];
                                plugin.urlChecked(results, packageName);
                            }
                            catch(err) {
                                plugin.urlChecked(results, url.substring(url.lastIndexOf("/") + 1));
                            }
                        }
                        else {
                            plugin.error(qsTr("File not found"));
                        }
                    }
                    catch(err) {
                        plugin.error(err);
                    }
                }
                else {
                    // Image
                    try {
                        if (settings.retrieveGallery) {
                            // Try to retrieve the gallery
                            var galleryLink = /\/g\/\w+/.exec(request.responseText);
                            
                            if (galleryLink) {
                                plugin.checkUrl("http://imgbox.com" + galleryLink);
                                return;
                            }
                        }

                        var container = request.responseText.split("class=\"image-content\"")[1].split("/>")[0];
                        var link = container.split("src=\"")[1].split("\"")[0];

                        if (link) {
                            var fileName = container.split("title=\"")[1].split("\"")[0];

                            if (!fileName) {
                                fileName = link.substring(link.lastIndexOf("/") + 1);
                            }
                            
                            plugin.urlChecked(new UrlResult(url, fileName));
                        }
                        else {
                            plugin.error(qsTr("File not found"));
                        }
                    }
                    catch(err) {
                        plugin.error(err);
                    }
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var container = request.responseText.split("class=\"image-content\"")[1].split("/>")[0];
                    var link = container.split("src=\"")[1].split("\"")[0];

                    if (link) {
                        plugin.downloadRequest(new NetworkRequest(link));
                    }
                    else {
                        plugin.error(qsTr("File not found"));
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.cancelCurrentOperation = function() {
        if (request) {
            request.abort();
            request = null;
        }

        return true;
    };

    return plugin;
})
