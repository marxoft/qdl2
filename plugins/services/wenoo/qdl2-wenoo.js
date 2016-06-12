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

var FILENAME_REGEXP = /og:title" content="([^"]+)/;
var AUDIO_URL_REGEXP = /http(s|):\/\/wenoo\.net\/media\/files_flv\/user\d+\/\w+\.mp3/;
var IMAGE_URL_REGEXP = /http(s|):\/\/wenoo\.net\/media\/files_image\/user\d+\/\w+\.jpg/;
var VIDEO_URL_REGEXP = /http(s|):\/\/wenoo\.net\/media\/files_flv\/user\d+\/\w+\.mp4/;

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var fileName = FILENAME_REGEXP.exec(request.responseText)[1];
                
                if (fileName) {                    
                    if (/\/audio\//.test(url)) {
                        fileName += ".mp3";
                    }
                    else if (/\/image\//.test(url)) {
                        fileName += ".jpg";
                    }
                    else {
                        fileName += ".mp4";
                    }
                    
                    urlChecked({"url": url, "fileName": fileName});
                }
                else {
                    error("File not found");
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
    request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            try {
                var downloadUrl = "";
                
                if (/\/audio\//.test(url)) {
                    downloadUrl = AUDIO_URL_REGEXP.exec(request.responseText)[0];
                }
                else if (/\/image\//.test(url)) {
                    downloadUrl = IMAGE_URL_REGEXP.exec(request.responseText)[0];
                }
                else {
                    downloadUrl = VIDEO_URL_REGEXP.exec(request.responseText)[0];
                }
                
                if (downloadUrl) {
                    downloadRequest({"url": downloadUrl});
                }
                else {
                    error("Unknown error");
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
