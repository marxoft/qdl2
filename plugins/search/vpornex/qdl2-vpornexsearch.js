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
    var BASE_URL = "http://vpornex.com/";
    var request = null;

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var results = [];
                    var posts = response.substring(response.indexOf("<main id=\"main\""), response.indexOf("</main>"))
                        .split("<div id=\"post-");

                    for (var i = 1; i < posts.length; i++) {
                        var post = posts[i];

                        try {
                            var url = /href="([^"]+)"/.exec(post)[1];
                            var name = /title="([^"]+)"/.exec(post)[1];
                            var thumbnail = /src="([^"]+)"/.exec(post)[1];
                            var duration = /fa-clock-o"><\/i> (\d{2}:\d{2})/.exec(post)[1];
                            var html = "<a href='" + url + "'><img width='320' height='180' src='" + thumbnail
                                + "' /><p>Duration: " + duration + "</p>";
                            
                            results.push(new SearchResult(name, html, url));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    try {
                        var next = /<a class="current">\d+<\/a><\/li><li><a href='([^']+)/.exec(response)[1];
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
            getResults(BASE_URL + (query ? "?s=" + query : "?filter=date&cat=0"));
        }
        else {
            var query = {"type": "text", "label": qsTr("Search query"), "key": "query"};
            plugin.settingsRequest(qsTr("Choose search options"), [query], function(params) {
                getResults(BASE_URL + (params.query ? "?s=" + params.query : "?filter=date&cat=0"));
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
