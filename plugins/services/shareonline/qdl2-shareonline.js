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
    var fileId = "";
    var captchaId = "";
    var captchaPath = "";
    var waitTime = 0;
    var timerId = -1;
    var request = null;

    function getCaptchaChallenge() {
        plugin.captchaRequest("qdl2-googlerecaptcha", CaptchaType.Image, "6LdatrsSAAAAAHZrB70txiV5p-8Iv8BtVxlTtjKX",
            submitCaptchaResponse);
    }

    function submitCaptchaResponse(challenge, response) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;

                if (response == "0") {
                    getCaptchaChallenge();
                }
                else {
                    var url = atob(response);

                    if (url) {
                        timerId = setTimeout(function() { plugin.downloadRequest(new NetworkRequest(url)); },
                            waitTime);
                        plugin.waitRequest(waitTime);
                    }
                    else {
                        plugin.error(qsTr("Unknown error"));
                    }
                }
            }
        }
            
        request.open("POST", "http://www.share-online.biz/dl/" + fileId + "/free/captcha/" + captchaPath);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("captcha=" + captchaId + "&recaptcha_challenge_field=" + challenge
            + "&recaptcha_response_field=" + response);
    }

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText.split(";");
                    var status = response[1];

                    switch (status) {
                        case "OK":
                            plugin.urlChecked(new UrlResult(url, response[2]));
                            break;
                        case "DELETED":
                            plugin.error(qsTr("File deleted"));
                            break;
                        case "NOT FOUND":
                            plugin.error(qsTr("File not found"));
                            break;
                        default:
                            plugin.error(qsTr("Unknown error"));
                            break;
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }
        
        request.open("GET", "http://api.share-online.biz/linkcheck.php?links=" + url);
        request.send();
    };

    plugin.getDownloadRequest = function(url) {
        try {
            fileId = /(\/dl\/|id=)([A-Z0-9]+)/.exec(url)[2];
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
                    captchaId = atob(/var dl=['"]([^'"]+)/.exec(response)[1]).split("chk||")[1];
                    captchaPath = /var url=['"]([^'"]+)/.exec(response)[1].split("/").slice(-1);
                    waitTime = parseInt(/var wait=(\d+)/.exec(response)[1]) * 1000;
                    timerId = setTimeout(getCaptchaChallenge, waitTime);
                    plugin.waitRequest(waitTime);
                }
                catch(e) {
                    try {
                        var errorString = /<strong>([^<]+)/.exec(response.split("dl_failure")[1])[1];

                        if (errorString.indexOf("already in use") != -1) {
                            // Already downloading. Wait 10 min.
                            plugin.waitRequest(600000, true);
                        }
                        else {
                            plugin.error(errorString);
                        }
                    }
                    catch(e) {
                        plugin.error(e);
                    }
                }
            }
        }
        
        request.open("POST", "http://www.share-online.biz/dl/" + fileId + "/free/");
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("dl_free=1&choice=free");
    };

    plugin.cancelCurrentOperation = function() {
        if (request) {
            request.abort();
            request = null;
        }

        if (timerId != -1) {
            clearTimeout(timerId);
            timerId = -1;
        }
        
        return true;
    };

    return plugin;
})
