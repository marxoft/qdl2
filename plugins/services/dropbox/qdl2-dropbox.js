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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function() {
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        try {
            var fragment = url.substring(url.lastIndexOf("/") + 1);
            var fileName = decodeURIComponent(fragment.substring(0, fragment.search(/[\?&]/)));
            plugin.urlChecked(new UrlResult(url, fileName));
        }
        catch(err) {
            plugin.error(err);
        }
    };

    plugin.getDownloadRequest = function(url) {
        try {
            var i = url.lastIndexOf("/") + 1;
            var fragment = url.substring(i);
            
            if (fragment.indexOf("dl=0") != -1) {
                fragment = fragment.replace("dl=0", "dl=1");
            }
            else if (fragment.indexOf("&") != -1) {
                fragment += "&dl=1";
            }
            else {
                fragment += "?dl=1";
            }
            
            url = url.substring(0, i) + fragment;
            plugin.downloadRequest(new NetworkRequest(url));
        }
        catch(err) {
            plugin.error(err);
        }
    };

    plugin.cancelCurrentOperation = function() {
        return true;
    };

    return plugin;
})
