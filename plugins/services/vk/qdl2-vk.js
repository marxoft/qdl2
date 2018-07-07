/**
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

(function() {
    var USER_AGENT = "Mozilla/5.0 (X11; Linux x86_64; rv:59.0) Gecko/20100101 Firefox/59.0";
    var VIDEO_FORMATS = ["url1080", "url720", "url480", "url360", "url240"];
    var request = null;
    var plugin = new ServicePlugin();

    function getVideoJSON(str) {
        try {
            var json = JSON.parse(str.split("<!json>")[1].split("<!>")[0]);
            return json.player.params[0];
        }
        catch(e) {
            return null;
        }
    }

    function getVideosJSON(str) {
        try {
            var json = JSON.parse(str.split("<!json>")[1].split("<!>")[0]);
            return json;
        }
        catch(e) {
            return null;
        }
    }

    function getVideoInfoUrl(url) {
        try {
            return "https://vk.com/al_video.php?act=show&al=1&module=community_videos&video=" + url.split("/video")[1];
        }
        catch(e) {
            return null;
        }
    }

    function checkVideoUrl(url) {
        var infoUrl = getVideoInfoUrl(url);

        if (!infoUrl) {
            plugin.error(qsTr("No video found"));
            return;
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var json = getVideoJSON(request.responseText);

                    if (json) {
                        var fileName = decodeHtml(json["md_title"]) + ".mp4";
                        plugin.urlChecked(new UrlResult(url, fileName));
                    }
                    else {
                        plugin.error(qsTr("No video found"));
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", infoUrl);
        request.setRequestHeader("Referer", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }
    
    function getVideoDownloadRequest(url, settings) {
        var infoUrl = getVideoInfoUrl(url);

        if (!infoUrl) {
            plugin.error(qsTr("No video found"));
            return;
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var json = getVideoJSON(request.responseText);
                    var formats = [];

                    for (var i = 0; i < VIDEO_FORMATS.length; i++) {
                        var f = VIDEO_FORMATS[i];
                        var u = json[f];

                        if (u) {
                            formats.push({"label": f.substring(3) + "P", "value": u});
                        }
                    }

                    if (!formats.length) {
                        plugin.error(qsTr("No video formats found"));
                        return;
                    }
                    
                    var format = settings.videoFormat || "1080P";

                    if (settings.useDefaultVideoFormat) {
                        for (var i = 0; i < formats.length; i++) {
                            if (formats[i].label == format) {
                                plugin.downloadRequest(new NetworkRequest(formats[i].value));
                                return;
                            }
                        }

                        plugin.downloadRequest(new NetworkRequest(formats[0].value));
                    }
                    else {
                        plugin.settingsRequest(qsTr("Video format"), [{"type": "list", "label": qsTr("Video format"),
                            "key": "url", "value": formats[Math.max(0, formats.indexOf(format))].value,
                            "options": formats}], function (f) { plugin.downloadRequest(new NetworkRequest(f.url)); });
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", infoUrl);
        request.setRequestHeader("Referer", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }

    function checkVideosUrl(url, results) {
        var oid;

        try {
            oid = url.split("videos-")[1].split(/\?|&/)[0];
        }
        catch(e) {
            plugin.error(qsTr("No album id found"));
            return;
        }

        var section;

        try {
            section = url.split("section=")[1].split(/\?|&/)[0];
        }
        catch(e) {
            section = "all";
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    if (!results) {
                        results = [];
                    }

                    var count = results.length;
                    var videos = getVideosJSON(request.responseText)[section].list;

                    for (var i = 0; i < videos.length; i++) {
                        try {
                            var video = videos[i];
                            var videoUrl = "https://vk.com/video" + video[0].toString() + "_" + video[1].toString();
                            var fileName = video[3] + ".mp4";
                            results.push(new UrlResult(videoUrl, fileName));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    if (results.length > count) {
                        checkVideosUrl(url, results);
                    }
                    else if (results.length) {
                        plugin.urlChecked(results, oid);
                    }
                    else {
                        plugin.error(qsTr("No videos found"));
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("POST", "https://vk.com/al_video.php");
        request.setRequestHeader("Referer", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send("act=load_videos_silent&al=1&need_albums=0&rowlen=3&snippet_video=0&oid=-" + oid + "&section="
            + section + "&offset=" + (results ? results.length : 0));
    }

    function getPhotoUrl(str) {
        try {
            var photoId = /"photo":"([^"]+)/.exec(str)[1];
            var json = str.split("{\"id\":\"" + photoId)[1].split("{\"id\"")[0];
            var url = json.split(/"[a-z]_src":"/g).pop().split("\"")[0].replace(/\\/g, "");
            return url;
        }
        catch(e) {
            return null;
        }
    }

    function checkPhotoUrl(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var photoUrl = getPhotoUrl(request.responseText);

                    if (photoUrl) {
                        var fileName = url.split("photo-")[1].split("?")[0] + ".jpg";
                        plugin.urlChecked(new UrlResult(url, fileName));
                    }
                    else {
                        plugin.error(qsTr("No photo found"));
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }

    function getPhotoDownloadRequest(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var photoUrl = getPhotoUrl(request.responseText);

                    if (photoUrl) {
                        plugin.downloadRequest(new NetworkRequest(photoUrl));
                    }
                    else {
                        plugin.error(qsTr("No photo found"));
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }

    function checkAlbumUrl(url, results) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    if (!results) {
                        results = [];
                    }

                    var count = results.length;
                    var response = request.responseText;
                    var re = /href="(\/photo-\d+_\d+)/g;
                    var match;

                    while (match = re.exec(response)) {
                        try {
                            var photoUrl = "https://vk.com" + match[1];
                            var fileName = photoUrl.substring(photoUrl.lastIndexOf("-") + 1) + ".jpg";
                            results.push(new UrlResult(photoUrl, fileName));
                            re.lastIndex = match.index + match[0].length;
                        }
                        catch(e) {
                            re.lastIndex = match.index + match[0].length;
                        }
                    }

                    if (results.length > count) {
                        checkAlbumUrl(url, results);
                    }
                    else if (results.length) {
                        var packageName;

                        try {
                            packageName = url.split("album-")[1].split("?")[0];
                        }
                        catch(e) {
                            packageName = qsTr("Unknown album");
                        }

                        plugin.urlChecked(results, packageName);
                    }
                    else {
                        plugin.error(qsTr("No photos found"));
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("POST", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send("al=1&part=1&offset=" + (results ? results.length : 0));
    }

    plugin.checkUrl = function(url) {
        if (url.indexOf("videos-") != -1) {
            checkVideosUrl(url);
        }
        else if (url.indexOf("album-") != -1) {
            checkAlbumUrl(url);
        }
        else if (url.indexOf("photo-") != -1) {
            checkPhotoUrl(url);
        }
        else {
            checkVideoUrl(url);
        }
    };

    plugin.getDownloadRequest = function(url, settings) {
        if (url.indexOf("photo-") != -1) {
            getPhotoDownloadRequest(url, settings);
        }
        else {
            getVideoDownloadRequest(url, settings);
        }
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
