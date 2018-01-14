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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var request = null;

    function submitCaptchaResponse(url, fileId, rand, challenge, response) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;

                try {
                    var downloadUrl = /href="([^"]+)">Direct Download Link/.exec(response)[1];
                    plugin.downloadRequest(new NetworkRequest(downloadUrl));
                }
                catch(e) {
                    try {
                        var errorString = /<div class="err"[^>]+>([^<]+)<\/div>/.exec(response)[1].trim();

                        if (errorString == "Wrong captcha") {
                            var rand = /"rand" value="([^"]+)"/.exec(response)[1];
                            var recaptchaKey = /challenge\.noscript\?k=([^"']+)/.exec(response)[1];
                            plugin.captchaRequest("qdl2-solvemediarecaptcha", CaptchaType.Image, recaptchaKey,
                                function(challenge, response) {
                                    submitCaptchaResponse(url, fileId, rand, challenge, response);
                                }
                            );
                        }
                        else {
                            plugin.error(errorString);
                        }
                    }
                    catch(e) {
                        plugin.error(qsTr("Unknown error"));
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

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;

                try {
                    var fileName = />Found<\/td><td>[^<]+<\/td><td>([^<]+)<\/td>/.exec(response)[1];
                    plugin.urlChecked(new UrlResult(url, fileName));
                }
                catch(e) {
                    if (response.indexOf("Not found!") != -1) {
                        plugin.error(qsTr("File not found"));
                    }
                    else {
                        plugin.error(e);
                    }
                }
            }
        }

        request.open("POST", "http://uploadrocket.net/?op=checkfiles");
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=checkfiles&process=Check+URLs&list=" + url);
    };

    plugin.getDownloadRequest = function(url) {
        try {
            var match = /(http(s|):\/\/(www\.|)uploadrocket\.net\/)([^\/\.]+)/.exec(url);
            var fileId = match[4];
            url = match[1] + fileId + ".htm";
        }
        catch(e) {
            plugin.error(qsTr("No file id found"));
            return;
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var rand = /"rand" value="([^"]+)"/.exec(response)[1];
                    var recaptchaKey = /challenge\.noscript\?k=([^"']+)/.exec(response)[1];
                    plugin.captchaRequest("qdl2-solvemediarecaptcha", CaptchaType.Image, recaptchaKey,
                        function (challenge, response) {
                            submitCaptchaResponse(url, fileId, rand, challenge, response);
                        }
                    );
                }
                catch(e) {
                    try {
                        var errorString = /<div class="err"[^>]+>([^<]+)<\/div>/.exec(response)[1].trim();
                        plugin.error(errorString);
                    }
                    catch(e) {
                        plugin.error(qsTr("Unknown error"));
                    }
                }
            }
        }

        request.open("POST", url);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=download1&usr_login=&referer=&method_isfree=method_isfree&Click+for+Free+Download&id="
            + fileId);
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
