/**
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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
        var request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var title = /html5player\.setVideoTitle\('([^']+)'\)/.exec(request.responseText)[1];

                    if (title) {
                        plugin.urlChecked(new UrlResult(url, title + ".mp4"));
                    }
                    else {
                        plugin.error(qsTr("File not found"));
                    }
                }
                catch(err) {
                    plugin.error(err);
                }
            }
        }

        request.open("GET", url);
        request.send();
    };

    plugin.getDownloadRequest = function(url, settings) {
        var request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                try {
                    var lowUrl = /html5player\.setVideoUrlLow\('([^']+)'\)/.exec(request.responseText)[1];
                    var highUrl = /html5player\.setVideoUrlHigh\('([^']+)'\)/.exec(request.responseText)[1];

                    if ((lowUrl) && (highUrl)) {
                        var format = settings.videoFormat || "mp4";

                        if (settings.useDefaultVideoFormat) {
                            if ((format == "mp4") && (highUrl)) {
                                plugin.downloadRequest(new NetworkRequest(highUrl));
                            }
                            else if (lowUrl) {
                                plugin.downloadRequest(new NetworkRequest(lowUrl));
                            }
                        }
                        else {
                            var settingsList = [];
                            var formatList = {"type": "list", "label": qsTr("Video format"), "key": "url"};
                            formatList["value"] = (format == "mp4" ? highUrl : lowUrl);
                            formatList["options"] = [{"label": "MP4", "value": highUrl},
                                                     {"label": "3GP", "value": lowUrl}];
                            plugin.settingsRequest(qsTr("Video format"), [formatList],
                                            function (f) { plugin.downloadRequest(new NetworkRequest(f.url)); });
                        }
                    }
                    else {
                        plugin.error(qsTr("No video streams found"));
                    }
                }
                catch(err) {
                    plugin.error(err)
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
