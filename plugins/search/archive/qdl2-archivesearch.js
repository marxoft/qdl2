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
    var DETAILS_URL = "https://archive.org/details/";
    var SEARCH_URL = "https://archive.org/advancedsearch.php?fl[]=description&fl[]=identifier&fl[]=mediatype&fl[]=title&sort[]=&sort[]=&sort[]=&rows=20&output=json";
    var request = null;

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var result = JSON.parse(request.responseText);
                    var results = [];

                    for (var i = 0; i < result.response.docs.length; i++) {
                        var doc = result.response.docs[i];
                        var html = "<p>" + doc.description + "</p><p>" + qsTr("Media type") + ": "
                            + doc.mediatype.substring(0, 1).toUpperCase() + doc.mediatype.substring(1) + "</p>";
                        results.push(new SearchResult(doc.title, html, DETAILS_URL + doc.identifier));
                    }

                    if (results.length) {
                        var params = result.responseHeader.params;
                        var next = SEARCH_URL + "&q=" + params.qin
                            + "&start=" + (parseInt(params.start) + parseInt(params.rows));
                        plugin.searchCompleted(results, {"url": next});
                    }
                    else {
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
            var query = settings.query || "";
            getResults(SEARCH_URL + "&q=" + query + "&page=" + page);
        }
        else {
            var query = {"type": "text", "label": qsTr("Search query"), "key": "query"};
            plugin.settingsRequest(qsTr("Choose search options"), [query], function(params) {
                getResults(SEARCH_URL + (params.query ? "&q=" + params.query : ""));
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

