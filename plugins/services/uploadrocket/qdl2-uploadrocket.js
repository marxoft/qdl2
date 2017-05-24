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

var request = null;

function checkUrl(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;

            try {
                var fileName = />Found<\/td><td>[^<]+<\/td><td>([^<]+)<\/td>/.exec(response)[1];
                urlChecked(new UrlResult(url, fileName));
            }
            catch(e) {
                if (response.indexOf("Not found!") != -1) {
                    error(qsTr("File not found"));
                }
                else {
                    error(e);
                }
            }
        }
    }

    request.open("POST", "http://uploadrocket.net/?op=checkfiles");
    request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    request.send("op=checkfiles&process=Check+URLs&list=" + url);
}

function getDownloadRequest(url) {
    try {
        var match = /(http(s|):\/\/(www\.|)uploadrocket\.net\/)([^\/\.]+)/.exec(url);
        var fileId = match[4];
        url = match[1] + fileId + ".htm";
    }
    catch(e) {
        error(qsTr("No file id found"));
        return;
    }

    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var response = request.responseText;
                var rand = /"rand" value="([^"]+)"/.exec(response)[1];
                var recaptchaKey = /challenge\.noscript\?k=([^"']+)/.exec(response)[1];
                captchaRequest("qdl2-solvemediarecaptcha", recaptchaKey, function (challenge, response) {
                    submitCaptchaResponse(url, fileId, rand, challenge, response);
                });
            }
            catch(e) {
                try {
                    var errorString = /<div class="err"[^>]+>([^<]+)<\/div>/.exec(response)[1].trim();
                    error(errorString);
                }
                catch(e) {
                    error(qsTr("Unknown error"));
                }
            }
        }
    }

    request.open("POST", url);
    request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    request.send("op=download1&usr_login=&referer=&method_isfree=method_isfree&Click+for+Free+Download&id=" + fileId);
}

function submitCaptchaResponse(url, fileId, rand, challenge, response) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;

            try {
                var downloadUrl = /href="([^"]+)">Direct Download Link/.exec(response)[1];
                downloadRequest(new NetworkRequest(downloadUrl));
            }
            catch(e) {
                try {
                    var errorString = /<div class="err"[^>]+>([^<]+)<\/div>/.exec(response)[1].trim();

                    if (errorString == "Wrong captcha") {
                        var rand = /"rand" value="([^"]+)"/.exec(response)[1];
                        var recaptchaKey = /challenge\.noscript\?k=([^"']+)/.exec(response)[1];
                        captchaRequest("qdl2-solvemediarecaptcha", recaptchaKey, function(challenge, response) {
                            submitCaptchaResponse(url, fileId, rand, challenge, response);
                        });
                    }
                    else {
                        error(errorString);
                    }
                }
                catch(e) {
                    error(qsTr("Unknown error"));
                }
            }
        }
    }

    request.open("POST", url);
    request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    request.send("op=download2&down_direct=1&method_isfree=method_isfree&method_ispremium=&id="
        + fileId + "&rand=" + rand + "&referer=" + url + "&adcopy_challenge=" + challenge
        + "&adcopy_response=" + response);
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}

