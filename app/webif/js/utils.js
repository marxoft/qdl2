/*
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

function formatDateTime(dt) {
    var day = dt.getDate();
    var month = dt.getMonth() + 1;
    var year = dt.getFullYear();
    var hour = dt.getHours();
    var min = dt.getMinutes();
    
    return (day < 10 ? "0" : "") + day + "/" + (month < 10 ? "0" : "") + month + "/" + year + " "
           + (hour < 10 ? "0" : "") + hour + ":" + (min < 10 ? "0" : "") + min;
}

function formatBytes(bytes) {
    var kb = 1024;
    var mb = kb * 1024;
    var gb = mb * 1024;
    
    if (bytes > gb) {
        return (bytes / gb).toFixed(1) + "GB";
    }
    
    if (bytes > mb) {
        return (bytes / mb).toFixed(1) + "MB";
    }
    
    if (bytes > kb) {
        return (bytes / kb).toFixed(1) + "KB";
    }
    
    return bytes.toFixed(1) + "B";
}
