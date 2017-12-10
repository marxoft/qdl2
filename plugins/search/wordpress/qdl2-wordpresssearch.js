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
    var POSTS_PATH = "/wp-json/wp/v2/posts";
    var USER_AGENT = "Mozilla/5.0 (X11; Linux x86_64; rv:53.0) Gecko/20100101 Firefox/53.0";
    var request = null;

    function getResults(params) {
        if (!params.host) {
            plugin.error(qsTr("No host specified"));
            return;
        }

        var host = params.host;

        if (host.substr(0, 4) != "http") {
            host = "http://" + host;
        }

        if (host[host.length - 1] == "/") {
            host = host.substr(0, host.length - 1);
        }

        if (!params.page) {
            params["page"] = 1;
        }

        var url = host + POSTS_PATH + "?per_page=20" + "&page=" + params.page + "&search=" + (params.search || "")
            + "&orderby=" + (params.orderby || "date") + "&order=" + (params.order || "desc");
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var posts = JSON.parse(request.responseText);

                    if (!posts.length) {
                        plugin.searchCompleted([]);
                        return;
                    }

                    var results = [];

                    for (var i = 0; i < posts.length; i++) {
                        var post = posts[i];

                        try {
                            results.push(new SearchResult(decodeHtml(post.title.rendered), post.content.rendered,
                                post.link));
                        }
                        catch(e) {
                            continue;
                        }
                    }

                    params["page"] = params.page + 1;
                    plugin.searchCompleted(results, params);
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }

    var plugin = new SearchPlugin();

    plugin.search = function(settings) {
        var h;

        if (settings.hosts) {
            var options = settings.hosts.split(/\s+/);
            h = {"type": "list", "label": qsTr("Host"), "key": "host", "value": options[0], "options": options};
        }
        else {
            h = {"type": "text", "label": qsTr("Host"), "key": "host"};
        }

        var s = {"type": "text", "label": qsTr("Search query (optional)"), "key": "search"};
        var ob = {"type": "list", "label": qsTr("Order by"), "key": "orderby", "value": "date",
            "options": [{"label": qsTr("Author"), "value": "author"}, {"label": qsTr("Date"), "value": "date"},
                {"label": qsTr("Title"), "value": "title"}]};
        var o = {"type": "list", "label": qsTr("Order"), "key": "order", "value": "desc",
            "options": [{"label": qsTr("Ascending"), "value": "asc"}, {"label": qsTr("Descending"), "value": "desc"}]};
        plugin.settingsRequest(qsTr("Choose search options"), [h, s, ob, o], function(params) {
            getResults(params);
        });
    };

    plugin.fetchMore = function(params) {
        getResults(params);
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
