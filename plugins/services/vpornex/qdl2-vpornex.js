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
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;
                    var fileName = decodeHtml(/itemprop="name">([^<]+)/.exec(response)[1]) + ".mp4";

                    try {
                        var cvUrl = /http(s|):\/\/(www\.|)cloudvideo\.tv\/(embed-[a-zA-Z0-9]+\.html|[a-zA-Z0-9]+(\.html|))/.exec(response)[0];
                        plugin.urlChecked(new UrlResult(cvUrl, fileName));
                    }
                    catch(e) {
                        try {
                            var rvUrl = /http(s|):\/\/(www\.|)rapidvideo\.com\/(\?v=|v\/|e\/)[0-9A-Z]+/.exec(response)[0];
                            plugin.urlChecked(new UrlResult(rvUrl, fileName));
                        }
                        catch(e) {
                            plugin.error(qsTr("No video URL found"));
                        }
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
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
