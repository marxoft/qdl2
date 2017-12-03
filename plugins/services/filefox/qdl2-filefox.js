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
    var rand = "";
    var timerId = -1;
    var request = null;

    function getFileName(response) {
        try {
            return /<i class="pfo pfo-dl1"><\/i>([^<]+)/.exec(request.responseText)[1].trim();
        }
        catch(e) {
            return null;
        }
    }

    function getFileId(response) {
        try {
            return /name="id" value="([^"]+)/.exec(response)[1];
        }
        catch(e) {
            return null;
        }
    }

    function getRand(response) {
        try {
            return /name="rand" value="([^"]+)/.exec(response)[1];
        }
        catch(e) {
            return null;
        }
    }

    function getWaitTime(response) {
        try {
            return parseInt(/class="time-remain">(\d+)/.exec(response)[1]) * 1000;
        }
        catch(e) {
            return 60000;
        }
    }

    function getLongWaitTime(errorString) {
        var hours = /(\d+) hours/.exec(errorString);
        var mins = /(\d+) minutes/.exec(errorString);
        var secs = /(\d+) seconds/.exec(errorString);
        var waitTime = 0;

        if (hours) {
            waitTime += parseInt(hours[1]) * 3600000;
        }

        if (mins) {
            waitTime += parseInt(mins[1]) * 60000;
        }

        if (secs) {
            waitTime += parseInt(secs[1]) * 1000;
        }

        if (!waitTime) {
            waitTime = 600000;
        }

        return waitTime;
    }

    function getDownloadUrl(response) {
        try {
            return /http(s|):\/\/\w\d+\.filefox\.cc\/[^'"]+/.exec(response)[0];
        }
        catch(e) {
            return null;
        }
    }

    function getErrorString(response) {
        try {
            return /class="row info-box info-box-blue text-center">([^<]+)/.exec(response)[1].trim();
        }
        catch(e) {
            return null;
        }
    }

    function getCaptchaChallenge() {
        plugin.captchaRequest("qdl2-googlerecaptcha", CaptchaType.Image, "6LetAu0SAAAAACCJkqZLvjNS4L7eSL8fGxr-Jzy2",
            submitCaptchaResponse);
    }

    function submitCaptchaResponse(challenge, response) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var downloadUrl = getDownloadUrl(response);

                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl));
                }
                else {
                    var errorString = getErrorString(response);

                    if (errorString) {
                        if (errorString.indexOf("Wrong Captcha") != -1) {
                            getCaptchaChallenge();
                        }
                        else {
                            plugin.error(errorString);
                        }
                    }
                    else {
                        plugin.error(qsTr("Unknown error"));
                    }
                }
            }
        }
            
        request.open("POST", "https://filefox.cc/" + fileId);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=download2&down_direct=1&id=" + fileId + "&rand=" + rand
            + "&recaptcha_challenge_field=" + challenge + "&recaptcha_response_field=" + response);
    }

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var fileName = getFileName(response);

                if (fileName) {
                    plugin.urlChecked(new UrlResult(url, fileName));
                }
                else {
                    var errorString = getErrorString(response);

                    if (errorString) {
                        plugin.error(errorString);
                    }
                    else {
                        plugin.error(qsTr("Unknown error"));
                    }
                }
            }
        }
        
        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var downloadUrl = getDownloadUrl(response);
                
                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl));
                }
                else {
                    fileId = getFileId(response);
                    rand = getRand(response);

                    if ((fileId) && (rand)) {
                        var waitTime = getWaitTime(response);
                        timerId = setTimeout(getCaptchaChallenge, waitTime);
                        plugin.waitRequest(waitTime);
                    }
                    else {
                        var errorString = getErrorString(response);

                        if (errorString) {
                            if (errorString.indexOf("Wait") != -1) {
                                plugin.waitRequest(getLongWaitTime(errorString), true);
                            }
                            else {
                                plugin.error(errorString);
                            }
                        }
                        else {
                            plugin.error(qsTr("Unknown error"));
                        }
                    }
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

        if (timerId != -1) {
            clearTimeout(timerId);
            timerId = -1;
        }
        
        return true;
    };

    return plugin;
})
