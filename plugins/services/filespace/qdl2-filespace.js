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
    var fileName = "";
    var fileUrl = "";
    var rand = "";
    var rand2 = "";
    var captchaKey = "";
    var timerId = -1;
    var request = null;

    function getRedirect(req) {
        var redirect = req.getResponseHeader("Location");

        if (redirect) {
            if (redirect[0] == "/") {
                redirect = "http://filespace.com" + redirect;
            }
        }

        return redirect;
    }

    function getFormValue(form, name) {
        try {
            return new RegExp("name=\"" + name + "\" value=\"([^\"]+)").exec(form)[1];
        }
        catch(e) {
            return null;
        }
    }

    function getWaitTime(str) {
        try {
            return parseInt(/Please wait <span[^>]+?>(\d+)/.exec(str)[1]) * 1000;
        }
        catch(e) {
            return 60000;
        }
    }

    function getLongWaitTime(str) {
        var hours = /(\d+) hours/.exec(str);
        var mins = /(\d+) minutes/.exec(str);
        var secs = /(\d+) seconds/.exec(str);
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

    function getDownloadUrl(str) {
        try {
            return /http(s|):\/\/[\w-_]+\.filespace\.com:\d+\/[^'"]+/.exec(str)[0];
        }
        catch(e) {
            return null;
        }
    }

    function getErrorString(str) {
        try {
            return /class="alert alert-danger"[^>]+?>([^<]+)/.exec(str)[1].trim();
        }
        catch(e) {
            return null;
        }
    }

    function getCaptchaKey(str) {
        try {
            return /api\.solvemedia\.com\/papi\/challenge\.noscript\?k=([^'"]+)/.exec(str)[1];
        }
        catch(e) {
            return null;
        }
    }

    function getCaptchaChallenge() {
        plugin.captchaRequest("qdl2-solvemediarecaptcha", CaptchaType.Image, captchaKey, submitCaptchaResponse);
    }

    function submitCaptchaResponse(challenge, response) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var downloadUrl = getDownloadUrl(getRedirect(request)) || getDownloadUrl(response);

                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl));
                }
                else if (response.indexOf("Wrong CAPTCHA") != -1) {
                    getCaptchaChallenge();
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

        request.followRedirects = false;
        request.open("POST", fileUrl);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=download2&method_free=Free+Download&down_script=1&id=" + fileId + "&fname=" + fileName
            + "&rand=" + rand + "&rand2=" + rand2 + "&adcopy_challenge=" + challenge + "&adcopy_response=" + response);
    }

    function getWaitRequest() {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                rand = getFormValue(response, "rand") || "";
                rand2 = getFormValue(response, "rand2") || "";
                captchaKey = getCaptchaKey(response);

                if (((rand) || (rand2)) && (captchaKey)) {
                    var waitTime = getWaitTime(response);
                    timerId = setTimeout(getCaptchaChallenge, waitTime);
                    plugin.waitRequest(waitTime);
                }
                else {
                    var errorString = getErrorString(response);

                    if (errorString) {
                        if (errorString.indexOf("wait") != -1) {
                            plugin.waitRequest(getLongWaitTime(errorString), true);
                        }
                        else if (errorString.indexOf("download") != -1) {
                            plugin.waitRequest(600000, true);
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

        request.open("POST", fileUrl);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=download1&method_free=Free+Download&id=" + fileId + "&fname=" + fileName);
    }

    function getLogin() {
        var username = {"type": "text", "label": qsTr("Username"), "key": "username"};
        var password = {"type": "password", "label": qsTr("Password"), "key": "password"};
        plugin.settingsRequest(qsTr("Login"), [username, password], function(params) {
            login(params.username, params.password);
        });
    }

    function login(username, password) {
        if ((!username) || (!password)) {
            plugin.error(qsTr("Invalid username or password"));
            return;
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                plugin.getDownloadRequest(fileUrl);
            }
        }

        request.open("POST", "http://filespace.com");
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        request.send("op=login&login=" + username + "&password=" + password);
    }

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var fileName = getFormValue(response, "fname");

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

    plugin.getDownloadRequest = function(url, settings) {
        fileUrl = url;

        if ((settings) && (settings["Account/useLogin"])) {
            if ((settings["Account/username"]) && (settings["Account/password"])) {
                login(settings["Account/username"], settings["Account/password"]);
            }
            else {
                getLogin();
            }

            return;
        }

        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                var response = request.responseText;
                var downloadUrl = getDownloadUrl(getRedirect(request)) || getDownloadUrl(response);
                
                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl));
                }
                else {
                    fileId = getFormValue(response, "id");
                    fileName = getFormValue(response, "fname");

                    if ((fileId) && (fileName)) {
                        getWaitRequest();
                    }
                    else {
                        var errorString = getErrorString(response);

                        if (errorString) {
                            if (errorString.indexOf("wait") != -1) {
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
        
        request.followRedirects = false;
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
