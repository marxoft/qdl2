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
    var BASE_URL = "http://sexix.net/";
    var request = null;

    function formatDate(isodate) {
        return isodate.substring(0, isodate.indexOf("T"));
    }

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var results = [];
                    var posts = response.split("<div id=\"post-");

                    for (var i = 1; i < posts.length; i++) {
                        var post = posts[i];

                        try {
                            var url = /href="([^"]+)"/.exec(post)[1];
                            var name = decodeHtml(/title="([^"]+)"/.exec(post)[1]);
                            var thumbnail = /src="([^"]+)"/.exec(post)[1];
                            var date = formatDate(/datetime="([^"]+)"/.exec(post)[1]);
                            var html = "<a href='" + url + "'><img width='320' height='180' src='" + thumbnail
                                + "' /><p>Date: " + date + "</p>";
                            
                            results.push(new SearchResult(name, html, url));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    try {
                        var next = /href="([^"]+)">Next</.exec(response)[1].replace("&#038;", "&");
                        plugin.searchCompleted(results, {"url": next});
                    }
                    catch(e) {
                        plugin.searchCompleted(results);
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
    }

    var plugin = new SearchPlugin();

    plugin.search = function(settings) {
        if (settings.useDefault) {
            var query = settings.query;
            var order = settings.order || "rand";
            getResults(BASE_URL + "?orderby=" + order + (query ? "&s=" + query : ""));
        }
        else {
            var query = {"type": "text", "label": qsTr("Search query"), "key": "query"};
            var order = {"type": "list", "label": qsTr("Order by"), "key": "order", "value": "rand",
                "options": [{"label": qsTr("Random"), "value": "rand"},
                {"label": qsTr("Date"), "value": "date"}]};
            plugin.settingsRequest(qsTr("Choose search options"), [query, order], function(params) {
                getResults(BASE_URL + "?orderby=" + params.order + (params.query ? "&s=" + params.query : ""));
            });
        }
    };

    plugin.fetchMore = function(params) {
        getResults(params.url);
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
