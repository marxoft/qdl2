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

var qdl = new Qdl();
var currentTransfer = -1;
var currentSettingsTab = "generalSettings";

function init() {
    // toolBar
    document.getElementById("addUrlsButton").onclick = showAddUrlsDialog;
    document.getElementById("importUrlsButton").onclick = showImportUrlsDialog;
    document.getElementById("retrieveUrlsButton").onclick = showRetrieveUrlsDialog;
    document.getElementById("startButton").onclick = function () { qdl.startAllTransfers(); }
    document.getElementById("pauseButton").onclick = function () { qdl.pauseAllTransfers(); }
    document.getElementById("settingsButton").onclick = showSettingsDialog;
    document.getElementById("quitButton").onclick = function () {
        qdl.quit(function () { document.body.innerHTML = "QDL application has been closed"; });
    }
    
    document.getElementById("actionSelector").onchange = function () {
        qdl.setSettings({"nextAction": this.value});
    }

    // addUrlsDialog
    document.getElementById("addUrlsEdit").onchange = function () {
        document.getElementById("addUrlsOkButton").disabled = this.value == "";
    }
    
    document.getElementById("addUrlsCancelButton").onclick = cancelAddUrlsDialog;
    document.getElementById("addUrlsOkButton").onclick = function () {
        var urls = document.getElementById("addUrlsEdit").value.split(/\s+/);
        var service = document.getElementById("addUrlsServiceSelector").value;
        var category = document.getElementById("addUrlsCategorySelector").value;
        cancelAddUrlsDialog();
        qdl.addUrlChecks(urls, service, category, showUrlCheckDialog);
    }

    // retrieveUrlsDialog
    document.getElementById("retrieveUrlsEdit").onchange = function () {
        document.getElementById("retrieveUrlsAddButton").disabled = this.value == "";
    }
    
    document.getElementById("retrieveUrlsAddButton").onclick = function () {
        var edit = document.getElementById("retrieveUrlsEdit");
        var selector = document.getElementById("retrieveUrlsServiceSelector");
        qdl.addUrlRetrievals(edit.value.split(/\s+/), selector.value, getUrlRetrievals);
        edit.value = "";
    }
    
    document.getElementById("retrieveUrlsCancelButton").onclick = cancelRetrieveUrlsDialog;
    document.getElementById("retrieveUrlsOkButton").onclick = cancelRetrieveUrlsDialog;

    // urlCheckDialog
    document.getElementById("urlCheckCancelButton").onclick = cancelUrlCheckDialog;
    document.getElementById("urlCheckOkButton").onclick = cancelUrlCheckDialog;

    // settingsDialog
    document.getElementById("generalSettingsButton").onclick = showGeneralSettingsTab;
    document.getElementById("networkSettingsButton").onclick = showNetworkSettingsTab;
    document.getElementById("categorySettingsButton").onclick = showCategorySettingsTab;
    document.getElementById("serviceSettingsButton").onclick = showServiceSettingsTab;
    document.getElementById("recaptchaSettingsButton").onclick = showRecaptchaSettingsTab;
    document.getElementById("decaptchaSettingsButton").onclick = showDecaptchaSettingsTab;
    document.getElementById("settingsCancelButton").onclick = cancelSettingsDialog;
    document.getElementById("settingsSaveButton").onclick = function() {
        cancelSettingsDialog();
        //qdl.setSettings();
    }
    
    loadTransfers(0, 50);
}

function loadTransfers(offset, limit) {
    qdl.getTransfers(offset, limit, false, function (result) {
        document.getElementById("activeLabel").innerHTML = result.active + "DLs";
        document.getElementById("speedLabel").innerHTML = result.speedString;
        var transfers = result.transfers;
        
        for (var i = 0; i < transfers.length; i++) {
            appendTransfer(transfers[i]);
        }
    });
}

function showChildTransfers(index) {
    var table = document.getElementById("transfersTable");
    var row = table.rows[index];
    row.setAttribute("data-expanded", "true");
    qdl.getTransfer(row.id, true, function (transfer) {
        for (var i = 0; i < transfer.children.length; i++) {
            insertTransfer(++index, transfer.children[i]);
        }
    });
}

function hideChildTransfers(index) {
    var table = document.getElementById("transfersTable");
    var row = table.rows[index];
    row.setAttribute("data-expanded", "false");

    var i = index + 1;
    
    while (table.rows.length > 1) {
        if (table.rows[i].getAttribute("data-itemtype") == 2) {
            table.deleteRow(i);
        }
        else {
            break;
        }
    }
}

function toggleRowExpanded(index) {
    var table = document.getElementById("transfersTable");
    var row = table.rows[index];

    if (row.getAttribute("data-expanded") == "true") {
        hideChildTransfers(index);
    }
    else {
        showChildTransfers(index);
    }
}

function appendTransfer(transfer) {
    insertTransfer(-1, transfer);
}

function insertTransfer(index, transfer) {
    var table = document.getElementById("transfersTable");

    if (index == -1) {
        index = table.rows.length;
    }

    var row = table.insertRow(index);
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", transfer.id);
    row.setAttribute("title", transfer.name);
    row.setAttribute("data-current", "false");
    row.setAttribute("data-expanded", "false");
    row.setAttribute("data-itemtype", transfer.itemType);
    row.setAttribute("data-priority", transfer.priority);
    row.setAttribute("data-status", transfer.status);
    row.onclick = function () { setCurrentTransfer(index); }

    var col = row.insertCell(0);
    col.setAttribute("class", "TransferIconColumn");
    
    if (transfer.itemType == 1) {
        col.innerHTML = ">";
        col.onclick = function () { toggleRowExpanded(index); }
        //col.appendChild(indicator);
    }
    else {
        var icon = document.createElement("img");
        icon.src = transfer.pluginIconPath;
        icon.width = 16;
        icon.height = 16;
        col.appendChild(icon);
    }

    col = row.insertCell(1);
    col.setAttribute("class", "TransferNameColumn NoWrap");
    col.innerHTML = transfer.name;
    
    col = row.insertCell(2);
    col.setAttribute("class", "TransferPriorityColumn NoWrap");
    col.innerHTML = transfer.priorityString;

    col = row.insertCell(3);
    col.setAttribute("class", "TransferProgressColumn");
    var progressBar = document.createElement("div");
    progressBar.setAttribute("class", "ProgressBar");
    var label = document.createElement("div");
    label.setAttribute("class", "ProgressBarLabel");
    label.innerHTML = transfer.progressString;
    var fill = document.createElement("div");
    fill.setAttribute("class", "ProgressBarFill");
    fill.style.width = transfer.progress + "%";
    progressBar.appendChild(label);
    progressBar.appendChild(fill);
    col.appendChild(progressBar);

    col = row.insertCell(4);
    col.setAttribute("class", "TransferSpeedColumn NoWrap");

    if (transfer.itemType == 2) {
        col.innerHTML = transfer.speedString;
    }

    col = row.insertCell(5);
    col.setAttribute("class", "TransferStatusColumn NoWrap");
    col.innerHTML = transfer.statusString;
}

function setCurrentTransfer(index) {
    if (index == currentTransfer) {
        return;
    }

    var table = document.getElementById("transfersTable");

    if (currentTransfer != -1) {
        table.rows[currentTransfer].setAttribute("data-current", "false");
    }

    currentTransfer = index;
    table.rows[index].setAttribute("data-current", "true");
}

function showAddUrlsDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("addUrlsDialog").style.display = "block";
    var edit = document.getElementById("addUrlsEdit");
    edit.value = "";
    edit.focus();
    qdl.getSettings(["checkUrls", "defaultCategory", "defaultServicePlugin"], function (settings) {
        qdl.getServicePlugins(function (services) {
            document.getElementById("addUrlsCheckBox").checked = settings.checkUrls;
            var selector = document.getElementById("addUrlsServiceSelector");
            
            for (var i = selector.length - 1; i >= 1; i--) {
                selector.remove(i);
            }
            
            for (var i = 0; i < services.length; i++) {
                var option = document.createElement("option");
                option.text = services[i].displayName;
                option.value = services[i].id;
                selector.add(option);
            }

            selector.value = settings.defaultServicePlugin;
        });
        
        qdl.getCategories(function (categories) {
            var selector = document.getElementById("addUrlsCategorySelector");
            
            for (var i = selector.length - 1; i >= 1; i--) {
                selector.remove(i);
            }

            var categorySet = false;
            
            for (var i = 0; i < categories.length; i++) {
                var option = document.createElement("option");
                option.text = categories[i].name;
                option.value = categories[i].name;
                selector.add(option);
            }

            selector.value = settings.defaultCategory;
        });
    });
}

function cancelAddUrlsDialog() {
    document.getElementById("addUrlsDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function showImportUrlsDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("addUrlsDialog").style.display = "block";
}

function cancelImportUrlsDialog() {
    document.getElementById("addUrlsDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function showRetrieveUrlsDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("retrieveUrlsDialog").style.display = "block";
    var edit = document.getElementById("retrieveUrlsEdit");
    edit.value = "";
    edit.focus();
    qdl.getSettings("defaultServicePlugin", function (settings) {
        qdl.getServicePlugins(function (services) {
            var selector = document.getElementById("retrieveUrlsServiceSelector");
            
            for (var i = selector.length - 1; i >= 1; i--) {
                selector.remove(i);
            }
            
            for (var i = 0; i < services.length; i++) {
                var option = document.createElement("option");
                option.text = services[i].displayName;
                option.value = services[i].id;
                selector.add(option);
            }

            selector.value = settings.defaultServicePlugin;
        });
    });

    getUrlRetrievals();    
}

function cancelRetrieveUrlsDialog() {
    document.getElementById("retrieveUrlsDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
    qdl.clearUrlRetrievals();
}

function getUrlRetrievals() {
    qdl.getUrlRetrievals(function (result) {
        var progress = result.progress + "%";
        document.getElementById("retrieveUrlsProgressBarLabel").innerHTML = progress;
        document.getElementById("retrieveUrlsProgressBarFill").style.width = progress;
        document.getElementById("retrieveUrlsStatusLabel").innerHTML = result.statusString;
        document.getElementById("retrieveUrlsOkButton").disabled = result.status <= 1;

        if (result.status == 1) {
            setTimeout(getUrlRetrievals, 3000);
        }
    });
}

function showUrlCheckDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("urlCheckDialog").style.display = "block";
    getUrlChecks();
}

function cancelUrlCheckDialog() {
    document.getElementById("urlCheckDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
    qdl.clearUrlChecks();
}

function getUrlChecks() {
    qdl.getUrlChecks(function (result) {
        var table = document.getElementById("urlCheckTable");

        for (var i = table.rows.length - 1; i > 0; i--) {
            table.deleteRow(i);
        }

        var checks = result.checks;

        for (var i = 0; i < checks.length; i++) {
            var row = table.insertRow(-1);
            var col = row.insertCell(0);
            col.innerHTML = checks[i].url;
            col = row.insertCell(1);

            if (checks[i].checked) {
                col.innerHTML = (checks[i].ok ? "Y" : "N");
            }
        }

        var progress = result.progress + "%";
        document.getElementById("urlCheckProgressBarLabel").innerHTML = progress;
        document.getElementById("urlCheckProgressBarFill").style.width = progress;
        document.getElementById("urlCheckStatusLabel").innerHTML = result.statusString;
        document.getElementById("urlCheckCancelButton").disabled = result.status != 1;
        document.getElementById("urlCheckOkButton").disabled = result.status <= 1;

        if (result.status == 1) {
            setTimeout(getUrlChecks, 3000);
        }
    });
}

function showSettingsDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("settingsDialog").style.display = "block";
}

function cancelSettingsDialog() {
    document.getElementById("settingsDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function showGeneralSettingsTab() {
    if (currentSettingsTab != "generalSettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("generalSettingsButton").setAttribute("data-current", "true");
        document.getElementById("generalSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "generalSettings";
    }
}

function showNetworkSettingsTab() {
    if (currentSettingsTab != "networkSettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("networkSettingsButton").setAttribute("data-current", "true");
        document.getElementById("networkSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "networkSettings";
    }
}

function showCategorySettingsTab() {
    if (currentSettingsTab != "categorySettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("categorySettingsButton").setAttribute("data-current", "true");
        document.getElementById("categorySettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "categorySettings";
        var table = document.getElementById("categoriesTable");

        if (table.rows.length == 1) {
            qdl.getCategories(function (categories) {
                for (var i = 0; i < categories.length; i++) {
                    var row = table.insertRow(-1);
                    row.setAttribute("class", "TableRow");
                    var col = row.insertCell(0);
                    col.setAttribute("class", "CategoryNameColumn");
                    col.innerHTML = categories[i].name;
                    col = row.insertCell(1);
                    col.setAttribute("class", "CategoryPathColumn");
                    col.innerHTML = categories[i].path;
                }
            });
        }
    }
}

function showServiceSettingsTab() {
    if (currentSettingsTab != "serviceSettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("serviceSettingsButton").setAttribute("data-current", "true");
        document.getElementById("serviceSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "serviceSettings";
        var table = document.getElementById("serviceTable");

        if (table.rows.length == 0) {
            qdl.getServicePlugins(function (plugins) {
                for (var i = 0; i < plugins.length; i++) {
                    var row = table.insertRow(-1);
                    row.setAttribute("class", "TableRow");
                    row.setAttribute("id", plugins[i].id);
                    var col = row.insertCell(0);
                    col.setAttribute("class", "ServiceIconColumn");
                    var icon = document.createElement("img");
                    icon.width = 16;
                    icon.height = 16;
                    icon.src = plugins[i].iconFilePath;
                    col.appendChild(icon);
                    col = row.insertCell(1);
                    col.setAttribute("class", "ServiceNameColumn");
                    col.innerHTML = plugins[i].displayName;
                }
            });
        }
    }
}

function showRecaptchaSettingsTab() {
    if (currentSettingsTab != "recaptchaSettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("recaptchaSettingsButton").setAttribute("data-current", "true");
        document.getElementById("recaptchaSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "recaptchaSettings";
        var table = document.getElementById("recaptchaTable");

        if (table.rows.length == 0) {
            qdl.getRecaptchaPlugins(function (plugins) {
                for (var i = 0; i < plugins.length; i++) {
                    var row = table.insertRow(-1);
                    row.setAttribute("class", "TableRow");
                    row.setAttribute("id", plugins[i].id);
                    var col = row.insertCell(0);
                    col.setAttribute("class", "RecaptchaIconColumn");
                    var icon = document.createElement("img");
                    icon.width = 16;
                    icon.height = 16;
                    icon.src = plugins[i].iconFilePath;
                    col.appendChild(icon);
                    col = row.insertCell(1);
                    col.setAttribute("class", "RecaptchaNameColumn");
                    col.innerHTML = plugins[i].displayName;
                }
            });
        }
    }
}

function showDecaptchaSettingsTab() {
    if (currentSettingsTab != "decaptchaSettings") {
        document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
        document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        document.getElementById("decaptchaSettingsButton").setAttribute("data-current", "true");
        document.getElementById("decaptchaSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "decaptchaSettings";
        var table = document.getElementById("decaptchaTable");

        if (table.rows.length == 0) {
            qdl.getDecaptchaPlugins(function (plugins) {
                for (var i = 0; i < plugins.length; i++) {
                    var row = table.insertRow(-1);
                    row.setAttribute("class", "TableRow");
                    row.setAttribute("id", plugins[i].id);
                    var col = row.insertCell(0);
                    col.setAttribute("class", "DecaptchaIconColumn");
                    var icon = document.createElement("img");
                    icon.width = 16;
                    icon.height = 16;
                    icon.src = plugins[i].iconFilePath;
                    col.appendChild(icon);
                    col = row.insertCell(1);
                    col.setAttribute("class", "DecaptchaNameColumn");
                    col.innerHTML = plugins[i].displayName;
                }
            });
        }
    }
}
