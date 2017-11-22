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
    var request = null;
    var plugin = new ServicePlugin();

    plugin.checkUrl = function(url) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var fileName = /"og:title" content="([^"]+)/.exec(request.responseText)[1].trim();
                    plugin.urlChecked(new UrlResult(url, fileName));
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                try {
                    var response = request.responseText;

                    try {
                        var sources = JSON.parse(/"sources": (\[[^\]]+\])/.exec(response)[1]);

                        if (!sources.length) {
                            plugin.error(qsTr("No video formats found"));
                            return;
                        }

                        if (settings.useDefaultFormat) {
                            var format = settings.format || "1080";

                            for (var i = sources.length - 1; i >= 0; i--) {
                                var source = sources[i];

                                if (source.res == format) {
                                    plugin.downloadRequest(new NetworkRequest(source.file));
                                    return;
                                }
                            }

                            plugin.downloadRequest(new NetworkRequest(sources[0].file));
                            return;
                        }

                        var options = [];

                        for (var i = sources.length - 1; i >= 0; i--) {
                            var source = sources[i];
                            options.push({"label": source.res + "P", "value": source.file});
                        }

                        plugin.settingsRequest(qsTr("Choose video format"), [{"type": "list", "key": "url",
                            "label": qsTr("Video format"), "options": options, "value": options[0].value}],
                            function (conf) { plugin.downloadRequest(new NetworkRequest(conf.url)); });
                    }
                    catch(e) {
                        var sources = response.split("<source ");

                        if (sources.length < 2) {
                            plugin.error(qsTr("No video formats found"));
                            return;
                        }

                        if (settings.useDefaultFormat) {
                            var format = settings.format || "1080";

                            for (var i = sources.length - 1; i > 0; i--) {
                                var source = sources[i];
                                var res = /data-res="([^"]+)/.exec(source)[1];

                                if (res == format) {
                                    var src = /src="([^"]+)/.exec(source)[1];

                                    if (src) {
                                        plugin.downloadRequest(new NetworkRequest(src));
                                        return;
                                    }
                                }
                            }

                            var src = /src="([^"]+)/.exec(sources[1])[1];

                            if (src) {
                                plugin.downloadRequest(new NetworkRequest(src));
                            }
                            else {
                                plugin.error(qsTr("No video formats found"));
                            }

                            return;
                        }

                        var options = [];

                        for (var i = sources.length - 1; i > 0; i--) {
                            var source = sources[i];
                            var res = /data-res="([^"]+)/.exec(source)[1];
                            var src = /src="([^"]+)/.exec(source)[1];

                            if ((res) && (src)) {
                                options.push({"label": res + "P", "value": src});
                            }
                        }

                        if (!options.length) {
                            plugin.error(qsTr("No video formats found"));
                            return;
                        }

                        plugin.settingsRequest(qsTr("Choose video format"), [{"type": "list", "key": "url",
                            "label": qsTr("Video format"), "options": options, "value": options[0].value}],
                            function (conf) { plugin.downloadRequest(new NetworkRequest(conf.url)); });
                    }
                }
                catch(e) {
                    plugin.error(e);
                }
            }
        }

        request.open("GET", url + "&q=0p");
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
