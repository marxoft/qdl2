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
    var BASE_URL = "https://www.bitporno.com";
    var request = null;

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var results = [];
                    var posts = response.split("<div class=\"entry");

                    for (var i = 1; i < posts.length; i++) {
                        var post = posts[i];

                        try {
                            var url = BASE_URL + /href="([^"]+)/.exec(post)[1];
                            var name = /<div style="font-weight[^>]+>([^<]+)/.exec(post)[1];
                            var thumbnail = /src="([^"]+)/.exec(post)[1];
                            var html = "<a href='" + url + "'><img width='320' height='180' src='" + thumbnail + "'/>";
                            results.push(new SearchResult(name, html, url));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    try {
                        var next = /<a href="([^"]+)" class="pages">Next<\/a>/.exec(response)[1];
                        plugin.searchCompleted(results, {"url": BASE_URL + decodeHtml(next)});
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

    function performSearch(params) {
        var url = BASE_URL + "/?q=" + (params.q || "") + "&sort=" + (params.sort || "newest") + "&time="
            + (params.time || "someday") + "&length=" + (params.length || "all");
        getResults(url);
    }

    var plugin = new SearchPlugin();

    plugin.search = function(settings) {
        if (settings.useDefault) {
            performSearch(settings);
        }
        else {
            var q = {"type": "text", "label": qsTr("Query"), "key": "q"};
            var s = {"type": "list", "label": qsTr("Sort"), "key": "sort", "value": "newest",
                "options": [{"label": qsTr("Newest"), "value": "newest"}, {"label": qsTr("Oldest"), "value": "oldest"},
                    {"label": qsTr("Most viewed"), "value": "mostviewed"}, {"label": qsTr("A-Z"), "value": "az"},
                    {"label": qsTr("Z-A"), "value": "za"}]};
            var t = {"type": "list", "label": qsTr("Time"), "key": "time", "value": "someday",
                "options": [{"label": qsTr("Anytime"), "value": "someday"}, {"label": qsTr("Today"), "value": "today"},
                    {"label": qsTr("This week"), "value": "tweek"}, {"label": qsTr("This month"), "value": "tmonth"},
                    {"label": qsTr("This year"), "value": "tyear"}, {"label": qsTr("Past"), "value": "past"}]};
            var l = {"type": "list", "label": qsTr("Duration"), "key": "length", "value": "all",
                "options": [{"label": qsTr("Any length"), "value": "all"}, {"label": qsTr("Short"), "value": "short"},
                    {"label": qsTr("Medium"), "value": "middle"}, {"label": qsTr("Long"), "value": "long"}]};
            plugin.settingsRequest(qsTr("Choose search options"), [q, s, t, l], function(params) {
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
