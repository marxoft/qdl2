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
    var RECENT_URL = "http://www.xopenload.com/recent";
    var SEARCH_URL = "http://www.xopenload.com/search.php";
    var request = null;

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var articles = response.split("<article class=\"item movies\"");
                    var results = [];

                    for (var i = 1; i < articles.length; i++) {
                        var article = articles[i];
                        var href = /href="([^"]+)"/.exec(article)[1];
                        var name = /alt="([^"]+)"/.exec(article)[1];
                        var thumbnail = /src="([^"]+)"/.exec(article)[1];
                        var category = /category\/\d+\/">([^<]+)</.exec(article)[1];
                        var date = /<span>(\d{2}\/\d{2}\/\d{4})<\/span>/.exec(article)[1];
                        var html = "<a href='" + href + "'><img width='212' height='300' src='" + thumbnail
                            + "' /><p>Category: " + category + "</p><p>Date: " + date + "</p>";
                        
                        results.push(new SearchResult(name, html, href));
                    }

                    try {
                        var pagination = response.substring(response.indexOf("class=\"pagination\""));
                        var next = /href="([^"]+)">Next/.exec(pagination)[1];
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
            getResults(query ? SEARCH_URL + "?s=" + query : RECENT_URL);
        }
        else {
            var query = {"type": "text", "label": qsTr("Search query"), "key": "query"};
            plugin.settingsRequest(qsTr("Choose search options"), [query], function (params) {
                getResults(params.query ? SEARCH_URL + "?s=" + params.query : RECENT_URL);
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
