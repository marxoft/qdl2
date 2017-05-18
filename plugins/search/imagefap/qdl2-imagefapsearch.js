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

var GALLERY_URL = "http://www.imagefap.com/gallery.php";

var request = null;

function getResults(url) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var response = request.responseText;
                var items = response.split("<tr id=\"");
                var results = [];
                
                for (var i = 1; i < items.length; i++) {
                    try {
                        var item = items[i];
                        var name = /<b>([^<]+)/.exec(item)[1];
                        var url = GALLERY_URL + "?gid=" + item.substring(0, item.indexOf("\""));
                        var count = /&nbsp;(\d+)&nbsp;/.exec(item)[1];
                        var quality = /alt="([^"]+)/.exec(item)[1];
                        var date = /&nbsp;(\d{4}-\d{2}-\d{2})&nbsp;/.exec(item)[1];
                        var thumbs = item.match(/<img\s+class="gal_thumb"[^>]+>/g);
                        var html = "<p>" + thumbs.join("</p><p>") + "<p>Images: " + count
                            + "</p><p>Quality: " + quality + "</p><p>Date: " + date + "</p>";
                        results.push(new SearchResult(name, html, url));
                    }
                    catch(err) {
                        print(err);
                    }
                }
                
                try {
                    var next = /href="([^"]+)">:: next ::/.exec(response)[1];
                    searchCompleted(results, {"url": next});
                }
                catch(err) {
                    searchCompleted(results);
                }
            }
            catch(err) {
                error(err);
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
    if (settings.value("useDefaultSearchOptions", false) === true) {
        var query = settings.value("query");
        var size = settings.value("size");
        var date = settings.value("date");
        getResults(GALLERY_URL + "?search=" + query + "&submitbutton=Search!&filter_size=" + size
        + "&filter_date=" + date);
    }
    else {
        var query = {"type": "text", "label": "Search query", "key": "query"};
        var size = {"type": "list", "label": "Size", "key": "size", "value": "",
            "options": [{"label": "Any size", "value": ""}, {"label": "Small", "value": "small"},
                {"label": "Medium", "value": "medium"}, {"label": "Large", "value": "large"},
                {"label": "Huge", "value": "huge"}]};
        var date = {"type": "list", "label": "Date", "key": "date", "value": "",
            "options": [{"label": "Anytime", "value": ""}, {"label": "Today", "value": "today"},
                {"label": "This week", "value": "week"}, {"label": "This month", "value": "month"}]};
        
        settingsRequest(qsTr("Choose search options"), [query, size, date], function(s) {
            getResults(GALLERY_URL + "?search=" + s.query + "&submitbutton=Search!&filter_size=" + s.size
            + "&filter_date=" + s.date);
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

