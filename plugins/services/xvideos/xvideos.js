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

function checkUrl(url) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var paramString = "[" + request.responseText.split("new HTML5Player(")[1]
                    .split(", video_related")[0].replace(/'/g, "\"") + "]";
                var list = JSON.parse(paramString);

                if ((list) && (list.length > 4)) {
                    urlChecked({"url": url, "fileName": list[2] + ".mp4"});
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

function getDownloadRequest(url) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var paramString = "[" + request.responseText.split("new HTML5Player(")[1]
                    .split(", video_related")[0].replace(/'/g, "\"") + "]";
                var list = JSON.parse(paramString);

                if ((list) && (list.length > 4)) {
                    var format = settings.value("videoFormat", "mp4");

                    if (settings.value("useDefaultVideoFormat", true) == true) {
                        if (format == "mp4") {
                            try {
                                downloadRequest({"url": list[5]});
                            }
                            catch(err) {
                                downloadRequest({"url": list[4]});
                            }
                        }
                        else {
                            downloadRequest({"url": list[4]});
                        }
                    }
                    else {
                        var settingsList = [];
                        var formatList = {"type": "list", "label": qsTr("Video &format"), "key": "url"};
                        
                        try {
                            formatList["value"] = (format == "mp4" ? list[5] : list[4]);
                            formatList["options"] = [{"label": "MP4", "value": list[5]},
                                                     {"label": "3GP", "value": list[4]}];
                        }
                        catch(err) {
                            formatList["value"] = list[4];
                            formatList["options"] = [{"label": "3GP", "value": list[4]}];
                        }

                        settingsRequest(qsTr("Video format"), [formatList],
                                        function (f) { downloadRequest({"url": f.url}); });
                    }
                }
                else {
                    error(qsTr("File not found"));
                }
            }
            catch(err) {
                error(err)
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
