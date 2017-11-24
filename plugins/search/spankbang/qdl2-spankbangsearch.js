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
    var BASE_URL = "https://m.spankbang.com";
    var PROTOCOL = "https:";
    var COOKIE = "mobile=on";
    var request = null;

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var results = [];
                    var posts = response.split("<div class=\"video-item\"");

                    for (var i = 1; i < posts.length; i++) {
                        var post = posts[i];

                        try {
                            var url = BASE_URL + /href="([^"]+)/.exec(post)[1];
                            var name = /<div class="inf">(.+)<\/div>/.exec(post)[1].replace(/<[^>]+>/g, "").trim();
                            var thumbnail = PROTOCOL + /src="([^"]+)/.exec(post)[1].replace("/200/", "/320/");
                            var duration = /class="i-len">([^<]+)/.exec(post)[1];
                            var quality;

                            try {
                                quality = /class="i-hd">([^<]+)/.exec(post)[1];
                            }
                            catch(e) {
                                quality = "SD";
                            }

                            var html = "<a href='" + url + "'><img width='320' height='180' src='" + thumbnail + "'/>"
                            + "<p>Quality: " + quality + "</p><p>Duration: " + duration + " min</p>";
                            results.push(new SearchResult(name, html, url));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    try {
                        var next = /class="next"><a href="([^"]+)/.exec(response)[1];
                        plugin.searchCompleted(results, {"url": BASE_URL + next});
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
        request.setRequestHeader("Cookie", COOKIE);
        request.send();
    }

    function performSearch(params) {
        var url = BASE_URL + "/s/" + (params.query || "") + "/?order=" + (params.order || "") + "&period="
            + (params.period || "");
        getResults(url);
    }

    var plugin = new SearchPlugin();

    plugin.search = function(settings) {
        if (settings.useDefault) {
            performSearch(settings);
        }
        else {
            var q = {"type": "text", "label": qsTr("Query"), "key": "query"};
            var s = {"type": "list", "label": qsTr("Sort by"), "key": "order", "value": "",
                "options": [{"label": qsTr("Relevant videos"), "value": ""},
                    {"label": qsTr("Popular videos"), "value": "top"}, {"label": qsTr("New videos"), "value": "new"},
                    {"label": qsTr("Most liked"), "value": "hot"}]};
            var t = {"type": "list", "label": qsTr("Time period"), "key": "period", "value": "",
                "options": [{"label": qsTr("Any time"), "value": ""}, {"label": qsTr("Today"), "value": "today"},
                    {"label": qsTr("This week"), "value": "week"}, {"label": qsTr("This month"), "value": "month"},
                    {"label": qsTr("This year"), "value": "year"}]};
            plugin.settingsRequest(qsTr("Choose search options"), [q, s, t], function(params) {
                performSearch(params);
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
