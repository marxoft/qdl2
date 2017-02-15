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

var CONFIG_URL = "http://www.porntrex.com/media/nuevo/config.php";
var FILE_NAME_REGEXP = /<title><\!\[CDATA\[(.+)\]\]><\/title>/
var VIDEO_ID_REGEXP = /\/video\/(\d+)/;
var VIDEO_FORMATS = ["filehd", "file"];
var VIDEO_FORMAT_NAMES = ["HD", "SD"];
var VIDEO_FORMAT_REGEXPS = [/<filehd>(.+)<\/filehd>/, /<file>(.+)<\/file>/];

var request = null;

function checkUrl(url) {
    try {
        var id = VIDEO_ID_REGEXP.exec(url)[1];
    }
    catch(err) {
        error(qsTr("Cannot parse video id from URL"));
        return;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var fileName = FILE_NAME_REGEXP.exec(request.responseText)[1];
                
                if (fileName) {
                    urlChecked(new UrlResult(url, fileName + ".mp4"));
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
    
    
    request.open("GET", CONFIG_URL + "?key=" + id);
    request.send();
}

function getDownloadRequest(url) {
    try {
        var id = VIDEO_ID_REGEXP.exec(url)[1];
    }
    catch(err) {
        error(qsTr("Cannot parse video id from URL"));
        return;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var format = settings.value("videoFormat", "filehd");

                if (settings.value("useDefaultVideoFormat", true) == true) {
                    for (var i = Math.max(0, VIDEO_FORMATS.indexOf(format)); i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = VIDEO_FORMAT_REGEXPS[i].exec(request.responseText)[1];
                            
                            if (videoUrl) {
                                downloadRequest(new NetworkRequest(videoUrl));
                                return;
                            }
                        }
                        catch(err) {
                            continue;
                        }
                    }
                }
                else {
                    var options = [];

                    for (var i = 0; i < VIDEO_FORMATS.length; i++) {
                        try {
                            var videoUrl = VIDEO_FORMAT_REGEXPS[i].exec(request.responseText)[1];
                            
                            if (videoUrl) {
                                options.push({"label": VIDEO_FORMAT_NAMES[i], "value": videoUrl});
                            }
                        }
                        catch(err) {
                            continue;
                        }
                    }

                    if (options.length > 0) {
                        settingsRequest(qsTr("Video format"), [{"type": "list", "label": qsTr("Video format"),
                                                          "key": "url",
                                                          "value": options[Math.max(0, options.indexOf(format))].value,
                                                          "options": options}],
                                        function (f) { downloadRequest(new NetworkRequest(f.url)); });
                        return;
                    }
                }

                error(qsTr("Unknown error"));
            }
            catch(err) {
                error(err);
            }
        }
    }

    request.open("GET", CONFIG_URL + "?key=" + id);
    request.send();
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
    }

    return true;
}
