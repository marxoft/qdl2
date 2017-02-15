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

var IMAGE_FORMATS = ["orig", "full", "share", "thumb"];
var VIDEO_FORMATS = ["orig", "mp4"];
var FILE_FORMATS = ["jpg", "mp4"];

var request = null;

function checkUrl(url) {
    if (FILE_FORMATS.indexOf(url.substring(url.lastIndexOf(".") + 1)) != -1) {
        urlChecked(new UrlResult(url, url.substring(url.lastIndexOf("/") + 1)));
        return;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var page = request.responseText;
                var items = getItems(page);

                if (!items) {
                    error(qsTr("No files found"));
                    return;
                }

                var type = settings.value("type", "");
                var images = [];
                var videos = [];
                var results = [];

                if ((!type) || (type == "Image")) {
                    images = getImages(items);
                }

                if ((!type) || (type == "Video")) {
                    videos = getVideos(items);
                }
                
                if (images.length > 0) {
                    var imageFormat = settings.value("imageFormat", "orig");

                    for (var i = 0; i < images.length; i++) {
                        var url = "";
                        var fileName = "";

                        for (var j = Math.max(0, IMAGE_FORMATS.indexOf(imageFormat)); j < IMAGE_FORMATS.length; j++) {
                            try {
                                url = images[i].urls[IMAGE_FORMATS[j]];
                                fileName = decodeURIComponent(images[i].name);
                            }
                            catch(err) {
                                continue;
                            }
                        }

                        if ((url) && (fileName)) {
                            results.push(new UrlResult(url, fileName));
                        }
                    }
                }
                    
                if (videos.length > 0) {
                    var videoFormat = settings.value("videoFormat", "orig");

                    for (var i = 0; i < videos.length; i++) {
                        var url = "";
                        var fileName = "";

                        for (var j = Math.max(0, VIDEO_FORMATS.indexOf(videoFormat)); j < VIDEO_FORMATS.length; j++) {
                            try {
                                url = videos[i].urls[VIDEO_FORMATS[j]];
                                fileName = decodeURIComponent(videos[i].name);
                            }
                            catch(err) {
                                continue;
                            }
                        }

                        if ((url) && (fileName)) {
                            results.push(new UrlResult(url, fileName));
                        }
                    }
                }
                
                if (results.length > 0) {
                    try {
                        urlChecked(results, page.split("class=\"title\">")[1].split("<")[0]);
                    }
                    catch(err) {
                        var packageName = results[0].fileName;
                        urlChecked(results, packageName.substring(0, packageName.lastIndexOf(".")));
                    }
                }
                else {
                    error(qsTr("No files found"));
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

function getDownloadRequest(url) {
    if (FILE_FORMATS.indexOf(url.substring(url.lastIndexOf(".") + 1)) != -1) {
        downloadRequest(new NetworkRequest(url));
        return;
    }

    error(qsTr("File not found"));
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
    }

    return true;
}

function getItems(page) {    
    try {
        return JSON.parse(page.split("var items = ")[1].split(";\n")[0]);
    }
    catch(err) {
        try {
            return JSON.parse(page.split("var album = ")[1].split(";\n")[0]).items;
        }
        catch(err) {
            return null;
        }
    }
}

function getImages(items) {
    var images = [];

    for (var i = 0; i < items.length; i++) {
        if (items[i].type == "Image") {
            images.push(items[i]);
        }
    }

    return images;
}

function getVideos(items) {
    var videos = [];

    for (var i = 0; i < items.length; i++) {
        if (items[i].type == "Video") {
            videos.push(items[i]);
        }
    }

    return videos;
}
