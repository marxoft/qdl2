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
    var API_URL = "https://api.mixcloud.com";
    var USER_AGENT = "Wget/1.13.4 (linux-gnu)";
    var request = null;

    function formatDate(iso) {
        try {
            var date = iso.substring(0, iso.indexOf("T")).split("-");
            return new Date(date[0], date[1] - 1, date[2]).toLocaleDateString();
        }
        catch(err) {
            return null;
        }
    }

    function formatDuration(secs) {
        if (secs > 0) {
            var time = new Date(secs * 1000).toTimeString();
            return time.substring(0, time.indexOf(" "));
        }
        
        return "--:--";
    }

    function getResults(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = JSON.parse(request.responseText);
                    var results = [];
                    
                    for (var i = 0; i < response.data.length; i++) {
                        try {
                            var item = response.data[i];
                            var name = item.name;
                            var url = item.url;
                            var thumbnailUrl = item.pictures.large;
                            var date = formatDate(item.created_time);
                            var duration = formatDuration(item.audio_length);
                            var html = "<a href='" + url + "'><img width='300' height='300' src='" + thumbnailUrl
                                + "' /><p>Date: " + date + "</p><p>Duration: " + duration + "</p>";
                            
                            results.push(new SearchResult(name, html, url));
                        }
                        catch(err) {
                            print(err);
                        }
                    }
                    
                    try {
                        var next = response.paging.next;
                        plugin.searchCompleted(results, {"url": next});
                    }
                    catch(err) {
                        plugin.searchCompleted(results);
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }

        request.open("GET", url);
        request.setRequestHeader("User-Agent", USER_AGENT);
        request.send();
    }

    var plugin = new SearchPlugin();

    plugin.search = function(settings) {
        if (settings.useDefaultSearchOptions) {
            var query = settings.searchQuery;
            getResults(API_URL + "/search/?q=" + query + "&type=cloudcast");
        }
        else {
            var query = {"type": "text", "label": qsTr("Search query"), "key": "searchQuery"};
            plugin.settingsRequest(qsTr("Choose search options"), [query], function(s) {
                getResults(API_URL + "/search/?q=" + s.searchQuery + "&type=cloudcast");
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
