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
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var links = response.split("<a class=\"link_a\"");

                    if (links.length < 2) {
                        plugin.error(qsTr("No files found"));
                        return;
                    }

                    var title = /<title>(.+) - Xopenload\.com<\/title>/.exec(response)[1];
                    var options = [];

                    for (var i = 1; i < links.length; i++) {
                        try {
                            var link = links[i];
                            var name = title + " (" + /<img[^>]+>([^<]+)</.exec(link)[1].trim() + ")";
                            var href = /href="([^"]+)"/.exec(link)[1];
                            options.push({"label": name, "value": href});
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    if (!options.length) {
                        plugin.error(qsTr("No files found"));
                        return;
                    }

                    var list = {"type": "list", "multiselect": true, "label": qsTr("Files"), "key": "urls",
                                "options": options};
                    plugin.settingsRequest(qsTr("Choose files"), [list], function(params) {
                        var urls = params.urls;

                        if (!urls.length) {
                            plugin.error(qsTr("No files chosen"));
                            return;
                        }

                        var results = [];
                        var getRedirect = function(url) {
                            request = new XMLHttpRequest();
                            request.onreadystatechange = function() {
                                if (request.readyState == 4) {
                                    try {
                                        var redirect =
                                            /window\.location\.href = "([^"]+)"/.exec(request.responseText)[1];
                                        results.push(new UrlResult(redirect, title + ".mp4"));

                                        if (urls.length) {
                                            getRedirect.call(plugin, urls.shift());
                                        }
                                        else {
                                            plugin.urlChecked(results, title);
                                        }
                                    }
                                    catch(e) {
                                        plugin.error(e);
                                    }
                                }
                            }

                            request.open("GET", url);
                            request.send();
                        };

                        getRedirect.call(plugin, urls.shift());
                    });
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
    }

    plugin.cancelCurrentOperation = function() {
        if (request) {
            request.abort();
            request = null;
        }

        return true;
    };

    return plugin;
})
