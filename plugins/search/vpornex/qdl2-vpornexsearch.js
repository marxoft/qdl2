/*!
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

var BASE_URL = "http://vpornex.com/";
var request = null;

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
                    var next = /<link rel="next" href="([^"]+)"/.exec(response)[1];
                    searchCompleted(results, {"url": next});
                }
                catch(e) {
                    searchCompleted(results);
                }
            }
            catch(e) {
                error(e);
            }
        }
    }

    request.open("GET", url);
    request.send();
}

function fetchMore(params) {
    getResults(params.url);
}

function search() {
    if (settings.value("useDefault", false) === true) {
        var query = settings.value("query");
        getResults(BASE_URL + (query ? "?s=" + query : ""));
    }
    else {
        var query = {"type": "text", "label": qsTr("Search query"), "key": "query"};
        settingsRequest(qsTr("Choose search options"), [query], function(params) {
            getResults(BASE_URL + (params.query ? "?s=" + params.query : ""));
        });
    }
}

function cancelCurrentOperation() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}

