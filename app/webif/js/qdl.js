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

var APPLICATION_PATH = "/app";
var CATEGORIES_PATH = "/categories";
var DECAPTCHA_PATH = "/decaptcha";
var RECAPTCHA_PATH = "/recaptcha";
var SERVICE_PATH = "/services";
var SETTINGS_PATH = "/settings"
var TRANSFERS_PATH = "/transfers";
var URLCHECK_PATH = "/urlcheck";
var URLRETRIEVAL_PATH = "/urlretrieval";

var Qdl = function (address, username, password) {
    this.ipaddress = address || "";
    this.username = username || "";
    this.password = password || "";
}

Qdl.prototype.del = function (path, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }    
    
    request.open("DELETE", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.send(null);
}

Qdl.prototype.get = function (path, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("GET", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.send(null);
}

Qdl.prototype.post = function (path, json_data, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 201) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("POST", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.setRequestHeader("Content-Type", "application/json");
    request.send(JSON.stringify(json_data));
}

Qdl.prototype.put = function (path, json_data, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("PUT", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.setRequestHeader("Content-Type", "application/json");
    request.send(JSON.stringify(json_data));
}

Qdl.prototype.quit = function (callback_ok, callback_error) {
    this.get(APPLICATION_PATH + "/quit", callback_ok, callback_error);
}

Qdl.prototype.showWindow = function (callback_ok, callback_error) {
    this.get(APPLICATION_PATH + "/showwindow", callback_ok, callback_error);
}

Qdl.prototype.closeWindow = function (callback_ok, callback_error) {
    this.get(APPLICATION_PATH + "/closewindow", callback_ok, callback_error);
}

Qdl.prototype.addTransfer = function (url, callback_ok, callback_error) {
    this.post(TRANSFERS_PATH, {"url": url}, callback_ok, callback_error);
}

Qdl.prototype.addTransfers = function (urls, callback_ok, callback_error) {
    this.post(TRANSFERS_PATH, {"urls": urls}, callback_ok, callback_error);
}

Qdl.prototype.getTransfers = function (offset, limit, includeChildren, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "?offset=" + offset + "&limit=" + limit + "&includeChildren=" + includeChildren,
             callback_ok, callback_error);
}

Qdl.prototype.getTransfer = function (id, includeChildren, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/" + id + "?includeChildren=" + includeChildren, callback_ok, callback_error);
}

Qdl.prototype.setTransferProperty = function (id, property, value, callback_ok, callback_error) {
    var propeties = {};
    properties[property] = value;
    this.put(TRANSFERS_PATH + "/" + id, properties, callback_ok, callback_error);
}

Qdl.prototype.setTransferProperties = function (id, properties, callback_ok, callback_error) {   
    this.put(TRANSFERS_PATH + "/" + id, properties, callback_ok, callback_error);
}

Qdl.prototype.startTransfer = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/start?id=" + id, callback_ok, callback_error);
}

Qdl.prototype.pauseTransfer = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/pause?id=" + id, callback_ok, callback_error);
}

Qdl.prototype.removeTransfer = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/remove?id=" + id, callback_ok, callback_error);
}

Qdl.prototype.moveTransfer = function (sourceId, destinationId, destinationRow, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/move?sourceId=" + sourceId + "&destinationId=" + destinationId
             + "&destinationRow=" + destinationRow, callback_ok, callback_error);
}

Qdl.prototype.startAllTransfers = function (callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/start", callback_ok, callback_error);
}

Qdl.prototype.pauseAllTransfers = function (callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/pause", callback_ok, callback_error);
}

Qdl.prototype.addCategory = function (name, path, callback_ok, callback_error) {
    this.post(CATEGORIES_PATH, {"name": name, "path": path}, callback_ok, callback_error);
}

Qdl.prototype.getCategories = function (callback_ok, callback_error) {
    this.get(CATEGORIES_PATH, callback_ok, callback_error);
}

Qdl.prototype.getCategory = function (name, callback_ok, callback_error) {
    this.get(CATEGORIES_PATH + "/" + name, callback_ok, callback_error);
}

Qdl.prototype.removeCategory = function (name, callback_ok, callback_error) {
    this.del(CATEGORIES_PATH + "/" + name, callback_ok, callback_error);
}

Qdl.prototype.getDecaptchaPlugins = function (callback_ok, callback_error) {
    this.get(DECAPTCHA_PATH, callback_ok, callback_error);
}

Qdl.prototype.getDecaptchaPlugin = function (id, callback_ok, callback_error) {
    this.get(DECAPTCHA_PATH + "/" + id, callback_ok, callback_error);
}

Qdl.prototype.getDecaptchaPluginSettings = function (id, callback_ok, callback_error) {
    this.get(DECAPTCHA_PATH + "/" + id + "/settings", callback_ok, callback_error);
}

Qdl.prototype.setDecaptchaPluginSettings = function (id, settings, callback_ok, callback_error) {
    this.put(DECAPTCHA_PATH + "/" + id + "/settings", settings, callback_ok, callback_error);
}

Qdl.prototype.getRecaptchaPlugins = function (callback_ok, callback_error) {
    this.get(RECAPTCHA_PATH, callback_ok, callback_error);
}

Qdl.prototype.getRecaptchaPlugin = function (id, callback_ok, callback_error) {
    this.get(RECAPTCHA_PATH + "/" + id, callback_ok, callback_error);
}

Qdl.prototype.getRecaptchaPluginSettings = function (id, callback_ok, callback_error) {
    this.get(RECAPTCHA_PATH + "/" + id + "/settings", callback_ok, callback_error);
}

Qdl.prototype.setRecaptchaPluginSettings = function (id, settings, callback_ok, callback_error) {
    this.put(RECAPTCHA_PATH + "/" + id + "/settings", settings, callback_ok, callback_error);
}

Qdl.prototype.getServicePlugins = function (callback_ok, callback_error) {
    this.get(SERVICE_PATH, callback_ok, callback_error);
}

Qdl.prototype.getServicePlugin = function (id, callback_ok, callback_error) {
    this.get(SERVICE_PATH + "/" + id, callback_ok, callback_error);
}

Qdl.prototype.getServicePluginSettings = function (id, callback_ok, callback_error) {
    this.get(SERVICE_PATH + "/" + id + "/settings", callback_ok, callback_error);
}

Qdl.prototype.setServicePluginSettings = function (id, settings, callback_ok, callback_error) {
    this.put(SERVICE_PATH + "/" + id + "/settings", settings, callback_ok, callback_error);
}

Qdl.prototype.getSettings = function (settings, callback_ok, callback_error) {
    this.get(SETTINGS_PATH + "?settings=" + settings.toString(), callback_ok, callback_error);
}

Qdl.prototype.setSettings = function (settings, callback_ok, callback_error) {
    this.put(SETTINGS_PATH, settings, callback_ok, callback_error);
}

Qdl.prototype.addUrlChecks = function (urls, pluginId, category, callback_ok, callback_error) {
    this.post(URLCHECK_PATH, {"urls": urls, "pluginId": pluginId, "category": category},
              callback_ok, callback_error);
}

Qdl.prototype.clearUrlChecks = function (callback_ok, callback_error) {
    this.del(URLCHECK_PATH, callback_ok, callback_error);
}

Qdl.prototype.getUrlChecks = function (callback_ok, callback_error) {
    this.get(URLCHECK_PATH, callback_ok, callback_error);
}

Qdl.prototype.getUrlCheck = function (url, callback_ok, callback_error) {
    this.get(URLCHECK_PATH + "/" + url, callback_ok, callback_error);
}

Qdl.prototype.removeUrlCheck = function (url, callback_ok, callback_error) {
    this.del(URLCHECK_PATH + "/" + url, callback_ok, callback_error);
}

Qdl.prototype.addUrlRetrievals = function (urls, pluginId, callback_ok, callback_error) {
    this.post(URLRETRIEVAL_PATH, {"urls": urls, "pluginId": pluginId}, callback_ok, callback_error);
}

Qdl.prototype.clearUrlRetrievals = function (callback_ok, callback_error) {
    this.del(URLRETRIEVAL_PATH, callback_ok, callback_error);
}

Qdl.prototype.getUrlRetrievals = function (callback_ok, callback_error) {
    this.get(URLRETRIEVAL_PATH, callback_ok, callback_error);
}

Qdl.prototype.getUrlRetrieval = function (url, callback_ok, callback_error) {
    this.get(URLRETRIEVAL_PATH + "/" + url, callback_ok, callback_error);
}

Qdl.prototype.removeUrlRetrieval = function (url, callback_ok, callback_error) {
    this.del(URLRETRIEVAL_PATH + "/" + url, callback_ok, callback_error);
}
