/**
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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var FILE_REGEXP = /http(s|):\/\/download\d+\.mediafire\.com\/[^'\"]+/;
    var FOLDER_INFO_URL = "http://www.mediafire.com/api/folder/get_info.php?response_format=json";
    var FOLDER_FILES_URL = "http://www.mediafire.com/api/folder/get_content.php?content_type=files&chunk_size=1000&response_format=json";

    var request = null;
    var recaptchaPluginId = "";
    var recaptchaKey = "";
    var recaptchaUrl = "";
    var securityKey = "";

    function checkFolder(folderId) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var response = JSON.parse(request.responseText).response;
                    var result = response.result;
                    
                    if (result == "Error") {
                        try {
                            plugin.error(response.message);
                        }
                        catch(err) {
                            plugin.error(err);
                        }
                    }
                    else {
                        var folder = response.folder_content;
                        var files = folder.files;
                        var results = [];
                        
                        for (var i = 0; i < files.length; i++) {
                            try {
                                results.push(new UrlResult(files[i].links.normal_download, files[i].filename));
                            }
                            catch(err) {
                                continue;
                            }
                        }
                        
                        if (results.length > 0) {
                            request.reset();
                            request.onreadystatechange = function () {
                                if (request.readyState == 4) {
                                    try {
                                        var packageName = JSON.parse(request.responseText).response.folder_info.name;
                                        plugin.urlChecked(results, packageName);
                                    }
                                    catch(err) {
                                        var fileName = results[0].fileName;
                                        plugin.urlChecked(results, fileName.substring(0, fileName.lastIndexOf(".")));
                                    }
                                }
                            }
                            
                            request.open("GET", FOLDER_INFO_URL + "&folder_key=" + folderId);
                            request.send();
                        }
                        else {
                            plugin.error(qsTr("No files found"));
                        }
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }
        
        request.open("GET", FOLDER_FILES_URL + "&folder_key=" + folderId);
        request.send();
    }

    function submitCaptchaResponse(challenge, response) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                var downloadUrl = FILE_REGEXP.exec(request.responseText);
                
                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl[0]));
                }
                else {
                    try {
                        recaptchaKey =
                            request.responseText.split("api.solvemedia.com/papi/challenge.script?k=")[1]
                            .split("\"")[0];
                        recaptchaPluginId = "qdl2-solvemediarecaptcha";
                        recaptchaUrl = "http://www.mediafire.com"
                            + request.responseText.split("form_captcha\" action=\"")[1].split("\"")[0];
                        securityKey = request.responseText.split("security\" value=\"")[1].split("\"")[0];
                        plugin.captchaRequest(recaptchaPluginId, CaptchaType.Image, recaptchaKey,
                            "submitCaptchaResponse");
                    }
                    catch(err) {
                        try {
                            recaptchaKey =
                                request.responseText.split("g-recaptcha\" data-sitekey=\"")[1].split("\"")[0];
                            recaptchaPluginId = "qdl2-googlerecaptcha";
                            recaptchaUrl = "http://www.mediafire.com"
                                + request.responseText.split("form_captcha\" action=\"")[1].split("\"")[0];
                            securityKey = request.responseText.split("security\" value=\"")[1].split("\"")[0];
                            plugin.captchaRequest(recaptchaPluginId, CaptchaType.Image, recaptchaKey,
                                "submitCaptchaResponse");
                        }
                        catch(err) {
                            plugin.error(err);
                        }
                    }
                }
            }
        }
            
        request.open("POST", recaptchaUrl);
        request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        
        if (recaptchaPluginId == "qdl2-googlerecaptcha") {
            request.send("recaptcha_challenge_field=" + challenge + "&recaptcha_response_field=" + response
                + "&security=" + securityKey);
        }
        else {
            request.send("adcopy_challenge=" + challenge + "&adcopy_response=" + response + "&security="
                + securityKey);
        }
    }

    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        if (url.indexOf("/folder/") != -1) {
            try {
                var folderId = /\/folder\/(\w+)/.exec(url)[1];
                checkFolder(folderId);
            }
            catch (err) {
                plugin.error(err);
            }
            
            return;
        }
        
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                if (request.responseText.indexOf("plugin.older_root") != -1) {
                    try {
                        var folderId = /\?(\w+)/.exec(url)[1];
                        checkFolder(folderId);
                    }
                    catch(err) {
                        plugin.error(err);
                    }
                }
                else {
                    try {
                        var fileName = request.responseText.split("og:title\" content=\"")[1].split("\"")[0];
                        plugin.urlChecked(new UrlResult(url, fileName));
                    }
                    catch(err) {
                        plugin.error(err);
                    }
                }
            }
        }
        
        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                var downloadUrl = FILE_REGEXP.exec(request.responseText);
                
                if (downloadUrl) {
                    plugin.downloadRequest(new NetworkRequest(downloadUrl[0]));
                }
                else {
                    try {
                        recaptchaKey =
                            request.responseText.split("api.solvemedia.com/papi/challenge.script?k=")[1]
                            .split("\"")[0];
                        recaptchaPluginId = "qdl2-solvemediarecaptcha";
                        recaptchaUrl = "http://www.mediafire.com"
                            + request.responseText.split("form_captcha\" action=\"")[1].split("\"")[0];
                        securityKey = request.responseText.split("security\" value=\"")[1].split("\"")[0];
                        plugin.captchaRequest(recaptchaPluginId, CaptchaType.Image, recaptchaKey,
                            "submitCaptchaResponse");
                    }
                    catch(err) {
                        try {
                            recaptchaKey =
                                request.responseText.split("g-recaptcha\" data-sitekey=\"")[1].split("\"")[0];
                            recaptchaPluginId = "qdl2-googlerecaptcha";
                            recaptchaUrl = "http://www.mediafire.com"
                                + request.responseText.split("form_captcha\" action=\"")[1].split("\"")[0];
                            securityKey = request.responseText.split("security\" value=\"")[1].split("\"")[0];
                            plugin.captchaRequest(recaptchaPluginId, CaptchaType.Image, recaptchaKey,
                                "submitCaptchaResponse");
                        }
                        catch(err) {
                            plugin.error(err);
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
        
        return true;
    };

    return plugin;
})
