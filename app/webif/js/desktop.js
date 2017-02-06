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
var currentTransfer = "";
var currentSettingsTab = "generalSettings";
var currentCategory = "";
var currentServicePlugin = "";
var currentRecaptchaPlugin = "";
var currentDecaptchaPlugin = "";
var requestId = "";
var requestedSettings = {};

function init() {
    // toolBar
    document.getElementById("addUrlsButton").onclick = showAddUrlsDialog;
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
    
    // packageMenu
    document.getElementById("packageReloadItem").onclick = function() {
        cancelPackageMenu();
        qdl.getTransfer(currentTransfer, true, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("packageStartItem").onclick = function () {
        cancelPackageMenu();
        qdl.startTransfer(currentTransfer, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("packagePauseItem").onclick = function () {
        cancelPackageMenu();
        qdl.pauseTransfer(currentTransfer, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("packageCancelItem").onclick = function () {
        cancelPackageMenu();
        
        if (window.confirm("Do you want to remove this package?"))  {
            qdl.removeTransfer(currentTransfer, false, function (transfer) {
                removeTransfer(document.getElementById(currentTransfer));
            });
        }
    }
    
    document.getElementById("packageCancelDeleteItem").onclick = function () {
        cancelPackageMenu();
        
        if (window.confirm("Do you want to remove this package and delete all files")) {
            qdl.removeTransfer(currentTransfer, true, function (transfer) {
                removeTransfer(document.getElementById(currentTransfer));
            });
        }
    }
    
    // transferMenu
    document.getElementById("transferReloadItem").onclick = function() {
        cancelTransferMenu();
        qdl.getTransfer(currentTransfer, false, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("transferStartItem").onclick = function () {
        cancelTransferMenu();
        qdl.startTransfer(currentTransfer, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("transferPauseItem").onclick = function () {
        cancelTransferMenu();
        qdl.pauseTransfer(currentTransfer, function (transfer) { updateTransfer(transfer); });
    }
    
    document.getElementById("transferCancelItem").onclick = function () {
        cancelTransferMenu();
        
        if (window.confirm("Do you want to remove this transfer?"))  {
            qdl.removeTransfer(currentTransfer, false, function (transfer) {
                removeTransfer(document.getElementById(currentTransfer));
            });
        }
    }
    
    document.getElementById("transferCancelDeleteItem").onclick = function () {
        cancelTransferMenu();
        
        if (window.confirm("Do you want to remove this transfer and delete all files")) {
            qdl.removeTransfer(currentTransfer, true, function (transfer) {
                removeTransfer(document.getElementById(currentTransfer));
            });
        }
    }
    
    // captchaDialog
    document.getElementById("captchaResponseEdit").onchange = function () {
        document.getElementById("captchaOkButton").disabled = this.value == "";
    }
    
    document.getElementById("captchaCancelButton").onclick = function () {
        cancelCaptchaDialog();
        qdl.setTransferProperty(requestId, "captchaImage", null, updateTransfer);
    }
    
    document.getElementById("captchaOkButton").onclick = function () {
        var response = document.getElementById("captchaResponseEdit").value;
        cancelCaptchaDialog();
        qdl.setTransferProperty(requestId, "captchaImage", response, updateTransfer);
    }
    
    // settingsRequestDialog
    document.getElementById("settingsRequestCancelButton").onclick = function () {
        cancelSettingsRequestDialog();
        qdl.setTransferProperty(requestId, "requestedSettings", null, updateTransfer);
    }
    
    document.getElementById("settingsRequestOkButton").onclick = function () {
        cancelSettingsRequestDialog();
        qdl.setTransferProperty(requestId, "requestedSettings", requestedSettings, updateTransfer);
    }

    // addUrlsDialog
    document.getElementById("addUrlsEdit").onchange = function () {
        document.getElementById("addUrlsOkButton").disabled = this.value == "";
    }
    
    document.getElementById("addUrlsCancelButton").onclick = cancelAddUrlsDialog;
    document.getElementById("addUrlsOkButton").onclick = function () {
        var urls = document.getElementById("addUrlsEdit").value.split(/\s+/);
        var category = document.getElementById("addUrlsCategorySelector").value;
        cancelAddUrlsDialog();
        qdl.addUrlChecks(urls, category, showUrlCheckDialog);
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
    
    // urlCheckDialog
    document.getElementById("urlCheckCancelButton").onclick = cancelUrlCheckDialog;
    document.getElementById("urlCheckOkButton").onclick = cancelUrlCheckDialog;

    // settingsDialog
    document.getElementById("generalSettingsButton").onclick = showGeneralSettingsTab;
    document.getElementById("serviceSettingsButton").onclick = showServiceSettingsTab;
    document.getElementById("recaptchaSettingsButton").onclick = showRecaptchaSettingsTab;
    document.getElementById("decaptchaSettingsButton").onclick = showDecaptchaSettingsTab;
    document.getElementById("settingsOkButton").onclick = function () {
        cancelSettingsDialog();
        var settings = {};
        settings["maximumConcurrentTransfers"] = document.getElementById("concurrentTransfersSelector").value;
        settings["startTransfersAutomatically"] = document.getElementById("automaticCheckBox").checked;
        settings["createSubfolders"] = document.getElementById("subfoldersCheckBox").checked;
        settings["extractArchives"] = document.getElementById("extractArchivesCheckBox").checked;
        settings["deleteExtractedArchives"] = document.getElementById("deleteArchivesCheckBox").checked;
        qdl.setSettings(settings);
    }
    
    loadTransfers(0, -1);
}

function loadTransfers(offset, limit) {
    qdl.getTransfers(offset, limit, false, function (result) {
        document.getElementById("activeLabel").innerHTML = result.active + "DLs";
        document.getElementById("speedLabel").innerHTML = result.speedString;
        var transfers = result.transfers;
        
        for (var i = 0; i < transfers.length; i++) {
            insertTransfer(transfers[i]);
        }
        
        checkForCaptchaAndSettingsRequest();
    });
}

function showChildTransfers(row) {
    row.setAttribute("data-expanded", "true");
    row.childNodes[0].innerHTML = "-";
    qdl.getTransfer(row.id, true, function (transfer) {
        for (var i = 0; i < transfer.children.length; i++) {
            row = insertTransfer(transfer.children[i], row.nextSibling);
        }
    });
}

function hideChildTransfers(row) {
    row.setAttribute("data-expanded", "false");
    row.childNodes[0].innerHTML = "+";    
    var next = row.nextSibling;
        
    while ((next) && (next.getAttribute("data-itemtype") == TransferItemType.TransferType)) {        
        row.parentNode.removeChild(next);
        next = row.nextSibling;
    }
}

function toggleRowExpanded(row) {
    if (row.getAttribute("data-expanded") == "true") {
        hideChildTransfers(row);
    }
    else {
        showChildTransfers(row);
    }
}

function insertTransfer(transfer, before) {
    var table = document.getElementById("transfersTable");
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", transfer.id);
    row.setAttribute("title", transfer.name);
    row.setAttribute("data-cancancel", transfer.canCancel);
    row.setAttribute("data-canpause", transfer.canPause);
    row.setAttribute("data-canstart", transfer.canStart);
    row.setAttribute("data-current", "false");
    row.setAttribute("data-expanded", "false");
    row.setAttribute("data-itemtype", transfer.itemType);
    row.setAttribute("data-priority", transfer.priority);
    row.setAttribute("data-status", transfer.status);
    row.onclick = function (event) { setCurrentTransfer(row); showMenu(event, row); }

    var col = document.createElement("div");
    col.setAttribute("class", "TransferIndicatorColumn");
    
    if (transfer.itemType == TransferItemType.PackageType) {
        col.innerHTML = "+";
        col.onclick = function (event) { event.stopPropagation(); toggleRowExpanded(row); }
    }
    else {
        col.innerHTML = "<br>";
    }
    
    row.appendChild(col);

    col = document.createElement("div");
    col.setAttribute("class", "TransferNameColumn NoWrap");
    
    if (transfer.itemType == TransferItemType.TransferType) {
        var icon = document.createElement("img");
        icon.setAttribute("class", "TransferIcon");
        icon.src = transfer.pluginIconPath;
        col.appendChild(icon);
        var name = document.createElement("div");
        name.setAttribute("class", "NoWrap");
        name.innerHTML = transfer.name;
        col.appendChild(name);
    }
    else {
        col.innerHTML = transfer.name;
    }
    
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "TransferPriorityColumn NoWrap");
    col.innerHTML = transfer.priorityString;
    row.appendChild(col);

    col = document.createElement("div");
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
    row.appendChild(col);

    col = document.createElement("div");
    col.setAttribute("class", "TransferSpeedColumn NoWrap");

    if (transfer.itemType == TransferItemType.TransferType) {
        col.innerHTML = transfer.speedString;
    }
    
    row.appendChild(col);

    col = document.createElement("div");
    col.setAttribute("class", "TransferStatusColumn NoWrap");
    col.innerHTML = transfer.statusString;
    row.appendChild(col);
    
    table.insertBefore(row, before);
    return row;
}

function removeTransfer(row) {
    if (row.getAttribute("data-expanded") == "true") {
        hideChildTransfers(row);
    }
    
    row.parentNode.removeChild(row);
}

function updateTransfer(transfer) {
    var row = document.getElementById(transfer.id);
        
    if (row) {
        row.setAttribute("title", transfer.name);
        row.setAttribute("data-priority", transfer.priority);
        row.setAttribute("data-status", transfer.status);
        
        if (transfer.itemType == TransferItemType.TransferType) {
            row.childNodes[1].childNodes[1].innerHTML = transfer.name;
            row.childNodes[4].innerHTML = transfer.speedString;
        }
        else {
            row.childNodes[1].innerHTML = transfer.name;
        }
        
        row.childNodes[2].innerHTML = transfer.priorityString;
        var progressBar = row.childNodes[3].childNodes[0];
        progressBar.childNodes[0].innerHTML = transfer.progressString;
        progressBar.childNodes[1].style.width = transfer.progress + "%";
        row.childNodes[5].innerHTML = transfer.statusString;
        
        if (transfer.itemType == TransferItemType.PackageType) {
            var children = transfer.children;
            
            if (children) {
                for (var i = 0; i < children.length; i++) {
                    updateTransfer(children[i]);
                }
            }
        }
    }
}

function setCurrentTransfer(row) {
    if (row.id == currentTransfer) {
        return;
    }
    
    if (currentTransfer) {
        var current = document.getElementById(currentTransfer);
        
        if (current) {
            current.setAttribute("data-current", "false");
        }
    }

    row.setAttribute("data-current", "true");
    currentTransfer = row.id;
}

function showMenu(event, row) {
    if (row.getAttribute("data-itemtype") == TransferItemType.PackageType) {
        showPackageMenu(event, row);
    }
    else {
        showTransferMenu(event, row);
    }
}

function showPackageMenu(event, row) {
    var background = document.getElementById("popupBackground");
    background.style.display = "block"
    background.onclick = cancelPackageMenu;
    document.getElementById("packageStartItem").style.display =
        (row.getAttribute("data-canstart") == "true" ? "block" : "none");
    document.getElementById("packagePauseItem").style.display =
        (row.getAttribute("data-canpause") == "true" ? "block" : "none");
    document.getElementById("packageCancelItem").style.display =
        document.getElementById("packageCancelDeleteItem").style.display =
        (row.getAttribute("data-cancancel") == "true" ? "block" : "none");
    var menu = document.getElementById("packageMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelPackageMenu() {
    document.getElementById("packageMenu").style.display = "none";
    var background = document.getElementById("popupBackground");
    background.style.display = "none"
    background.onclick = null;
}

function showTransferMenu(event, row) {
    var background = document.getElementById("popupBackground");
    background.style.display = "block"
    background.onclick = cancelTransferMenu;
    document.getElementById("transferStartItem").style.display =
        (row.getAttribute("data-canstart") == "true" ? "block" : "none");
    document.getElementById("transferPauseItem").style.display =
        (row.getAttribute("data-canpause") == "true" ? "block" : "none");
    document.getElementById("transferCancelItem").style.display =
        document.getElementById("transferCancelDeleteItem").style.display =
        (row.getAttribute("data-cancancel") == "true" ? "block" : "none");
    var menu = document.getElementById("transferMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelTransferMenu() {
    document.getElementById("transferMenu").style.display = "none";
    var background = document.getElementById("popupBackground");
    background.style.display = "none"
    background.onclick = null;
}

function checkForCaptchaAndSettingsRequest(status) {
    if (!status) {
        status = TransferItemStatus.AwaitingCaptchaResponse;
    }
    
    qdl.searchTransfers("status", status, 1, false, function (transfers) {
        if (status == TransferItemStatus.AwaitingCaptchaResponse) {
            if (transfers.length > 0) {
                showCaptchaDialog(transfers[0]);
            }
            else {
                checkForCaptchaAndSettingsRequest(TransferItemStatus.AwaitingDecaptchaSettingsResponse);
            }            
        }
        else if (transfers.length > 0) {
            showSettingsRequestDialog(transfers[0]);
        }
        else if (status < TransferItemStatus.AwaitingServiceSettingsResponse) {
            checkForCaptchaAndSettingsRequest(++status);
        }                           
    });
}

function showCaptchaDialog(transfer) {
    requestId = transfer.id;
    document.getElementById("popupBackground").style.display = "block";
    var dialog = document.getElementById("captchaDialog");
    dialog.style.display = "block"
    dialog.setAttribute("data-transferid", transfer.id);
    var image = document.getElementById("captchaImage");
    image.src = "data:image/jpeg;base64," + transfer.captchaImage;
    var edit = document.getElementById("captchaResponseEdit");
    edit.value = "";
    var label = document.getElementById("captchaLabel");
    label.innerHTML = formatMSecs(transfer.captchaTimeout);
}

function cancelCaptchaDialog() {
    document.getElementById("captchaDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function showSettingsRequestDialog(transfer) {
    requestId = transfer.id;
    requestedSettings = {};
    document.getElementById("popupBackground").style.display = "block";
    var dialog = document.getElementById("settingsRequestDialog");
    dialog.style.display = "block"
    dialog.setAttribute("data-transferid", transfer.id);
    var content = document.getElementById("settingsRequestDialogContent");
    
    for (var i = content.childNodes.length - 1; i >= 0; i--) {
        content.removeChild(content.childNodes[i]);
    }
    
    var settings = transfer.requestedSettings;
    
    for (var i = 0; i < settings.length; i++) {
        addSettingElement(content, settings[i], null, function (key, value) {
            requestedSettings[key] = value;
        });
    }
}

function cancelSettingsRequestDialog() {
    document.getElementById("settingsRequestDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function addSettingElement(element, setting, group, callback) {
    var key = setting.key;
    
    if (!key) {
        return;
    }
    
    if (group) {
        key = group + "/" + key;
    }
    
    requestedSettings[key] = setting.value;
    
    switch (setting.type) {
    case "boolean":
        addSettingCheckBox(element, setting.label, key, setting.value === true, callback);
        break;
    case "group":
        addSettingGroup(element, setting.label, key, setting.settings, callback);
        break;
    case "integer":
        addSettingIntegerSelector(element, setting.label, key, Math.max(0, parseInt(setting.minimum)),
                                  Math.max(1, parseInt(setting.maximum)), Math.max(1, parseInt(setting.maximum)),
                                  setting.value, callback);
        break;
    case "list":
        addSettingListSelector(element, setting.label, key, setting.options, setting.value, callback);
        break;
    case "password":
        addSettingPasswordField(element, setting.label, key, setting.value, callback);
        break;
    case "text":
        addSettingTextField(element, setting.label, key, setting.value, callback);
        break;
    default:
        break;
    }
}

function addSettingCheckBox(element, label, key, value, callback) {
    var checkBox = document.createElement("input");
    checkBox.setAttribute("class", "CheckBox");
    checkBox.setAttribute("id", "checkbox" + key);
    checkBox.setAttribute("type", "checkbox");
    checkBox.setAttribute("data-key", key);
    checkBox.checked = value;
    checkBox.onclick = function () { callback(key, this.checked); }
    var checkBoxLabel = document.createElement("label");
    checkBoxLabel.setAttribute("class", "NoWrap");
    checkBoxLabel.setAttribute("for", "checkbox" + key);
    checkBoxLabel.innerHTML = label;
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox");
    hbox.appendChild(checkBox);
    hbox.appendChild(checkBoxLabel);
    element.appendChild(hbox);
}

function addSettingGroup(element, label, key, settings, callback) {
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox NoWrap");
    hbox.innerHTML = label;
    element.appendChild(hbox);
    
    for (var i = 0; i < settings.length; i++) {
        addSettingElement(element, settings[i], key, callback);
    }
}

function addSettingIntegerSelector(element, label, key, minimum, maximum, step, value, callback) {
    var field = document.createElement("input");
    field.setAttribute("class", "TextField");
    field.setAttribute("id", "field" + key);
    field.setAttribute("type", "number");
    field.setAttribute("min", minimum);
    field.setAttribute("max", maximum);
    field.setAttribute("step", step);
    field.setAttribute("data-key", key);
    field.value = value;
    field.onchange = function () { callback(key, this.value); }
    var fieldLabel = document.createElement("label");
    fieldLabel.setAttribute("class", "NoWrap");
    fieldLabel.setAttribute("for", "field" + key);
    fieldLabel.innerHTML = label + ": ";
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox");
    hbox.appendChild(fieldLabel);
    hbox.appendChild(field);
    element.appendChild(hbox);
}

function addSettingListSelector(element, label, key, options, value, callback) {
    var selector = document.createElement("select");
    selector.setAttribute("class", "Selector");
    selector.setAttribute("id", "selector" + key);
    selector.setAttribute("data-key", key);
    
    for (var i = 0; i < options.length; i++) {
        var option = document.createElement("option");
        option.text = options[i].label;
        option.value = options[i].value;
        selector.add(option);
    }
    
    selector.value = value;
    selector.onchange = function () { callback(key, this.value); }
    var selectorLabel = document.createElement("label");
    selectorLabel.setAttribute("class", "NoWrap");
    selectorLabel.setAttribute("for", "selector" + key);
    selectorLabel.innerHTML = label + ": ";
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox");
    hbox.appendChild(selectorLabel);
    hbox.appendChild(selector);
    element.appendChild(hbox);
}

function addSettingPasswordField(element, label, key, value, callback) {
    var field = document.createElement("input");
    field.setAttribute("class", "TextField");
    field.setAttribute("id", "field" + key);
    field.setAttribute("type", "password");
    field.setAttribute("data-key", key);
    field.value = value;
    field.onchange = function () { callback(key, this.value); }
    var fieldLabel = document.createElement("label");
    fieldLabel.setAttribute("class", "NoWrap");
    fieldLabel.setAttribute("for", "field" + key);
    fieldLabel.innerHTML = label + ": ";
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox");
    hbox.appendChild(fieldLabel);
    hbox.appendChild(field);
    element.appendChild(hbox);
}

function addSettingTextField(element, label, key, value, callback) {
    var field = document.createElement("input");
    field.setAttribute("class", "TextField");
    field.setAttribute("id", "field" + key);
    field.setAttribute("type", "text");
    field.setAttribute("data-key", key);
    field.value = value;
    field.onchange = function () { callback(key, this.value); }
    var fieldLabel = document.createElement("label");
    fieldLabel.setAttribute("class", "NoWrap");
    fieldLabel.setAttribute("for", "field" + key);
    fieldLabel.innerHTML = label + ": ";
    var hbox = document.createElement("div");
    hbox.setAttribute("class", "HBox");
    hbox.appendChild(fieldLabel);
    hbox.appendChild(field);
    element.appendChild(hbox);
}

function showAddUrlsDialog(urls) {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("addUrlsDialog").style.display = "block";
    var edit = document.getElementById("addUrlsEdit");
    
    if (urls instanceof Array) {
        edit.value = urls.join("\n");
    }
    else {
        edit.value = "";
    }
    
    edit.focus();
    qdl.getSettings(["defaultCategory"], function (settings) {        
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
        
        if (result.status == UrlRetrievalStatus.Active) {
            setTimeout(getUrlRetrievals, 3000);
        }
        else {
            if (result.status == UrlRetrievalStatus.Completed) {
                var results = result.results;
                
                if (results.length > 0) {
                    showAddUrlsDialog(results);
                }
            }
            
            qdl.clearUrlRetrievals();
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

        for (var i = table.childNodes.length - 1; i > 0; i--) {
            table.removeChild(table.childNodes[i]);
        }

        var checks = result.checks;

        for (var i = 0; i < checks.length; i++) {
            var row = document.createElement("li");
            row.setAttribute("class", "TableRow");
            var col = document.createElement("div");
            col.setAttribute("class", "UrlCheckUrlColumn");
            col.innerHTML = checks[i].url;
            row.appendChild(col);
            col = document.createElement("div");
            col.setAttribute("class", "UrlCheckUrlColumn");
            
            if (checks[i].checked) {
                col.innerHTML = (checks[i].ok ? "Y" : "N");
            }
            
            row.appendChild(col);
        }

        var progress = result.progress + "%";
        document.getElementById("urlCheckProgressBarLabel").innerHTML = progress;
        document.getElementById("urlCheckProgressBarFill").style.width = progress;
        document.getElementById("urlCheckStatusLabel").innerHTML = result.statusString;
        document.getElementById("urlCheckCancelButton").disabled = result.status != UrlCheckStatus.Active;
        document.getElementById("urlCheckOkButton").disabled = result.status <= UrlCheckStatus.Active;

        if (result.status == UrlCheckStatus.Active) {
            setTimeout(getUrlChecks, 3000);
        }
    });
}

function showSettingsDialog() {
    document.getElementById("popupBackground").style.display = "block";
    document.getElementById("settingsDialog").style.display = "block";
    showGeneralSettingsTab();
    qdl.getSettings(["maximumConcurrentTransfers", "startTransfersAutomatically", "createSubfolders",
                     "extractArchives", "deleteExtractedArchives"], function (settings) {
                         document.getElementById("concurrentTransfersSelector").value =
                             settings.maximumConcurrentTransfers;
                         document.getElementById("automaticCheckBox").checked =
                             settings.startTransfersAutomatically === true;
                         document.getElementById("extractArchivesCheckBox").checked =
                             settings.extractArchives === true;
                         document.getElementById("deleteArchivesCheckBox").checked =
                             settings.deleteExtractedArchives === true;
                     });
}

function cancelSettingsDialog() {
    document.getElementById("settingsDialog").style.display = "none";
    document.getElementById("popupBackground").style.display = "none";
}

function showGeneralSettingsTab() {
    if (currentSettingsTab != "generalSettings") {
        if (currentSettingsTab) {
            document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
            document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        }
        
        document.getElementById("generalSettingsButton").setAttribute("data-current", "true");
        document.getElementById("generalSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "generalSettings";
    }
}

function showServiceSettingsTab() {
    if (currentSettingsTab != "serviceSettings") {
        if (currentSettingsTab) {
            document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
            document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        }
        
        document.getElementById("serviceSettingsButton").setAttribute("data-current", "true");
        document.getElementById("serviceSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "serviceSettings";
        var table = document.getElementById("serviceTable");

        if (table.childNodes.length == 0) {
            qdl.getServicePlugins(function (plugins) {                
                for (var i = 0; i < plugins.length; i++) {
                    insertServicePlugin(plugins[i]);
                }
            });
        }
    }
}

function insertServicePlugin(plugin, before) {
    var table = document.getElementById("serviceTable");
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", plugin.id);
    row.setAttribute("title", plugin.displayName);
    row.setAttribute("data-current", "false");
    row.onclick = function () { setCurrentServicePlugin(row); }
    var icon = document.createElement("img");
    icon.setAttribute("class", "PluginIcon");
    icon.src = plugin.iconFilePath;
    row.appendChild(icon);
    var name = document.createElement("div");
    name.setAttribute("class", "NoWrap");
    name.innerHTML = plugin.displayName;
    row.appendChild(icon);
    row.appendChild(name);
    table.insertBefore(row, before);
    return row;
}

function setCurrentServicePlugin(row) {
    if (row.id == currentServicePlugin) {
        return;
    }
    
    if (currentServicePlugin) {
        var current = document.getElementById(currentServicePlugin);
        
        if (current) {
            current.setAttribute("data-current", "false");
        }
    }

    row.setAttribute("data-current", "true");
    currentServicePlugin = row.id;
    var container = document.getElementById("serviceSettings");
    
    for (var i = container.childNodes.length - 1; i >= 0; i--) {
        container.removeChild(container.childNodes[i]);
    }
    
    qdl.getServicePluginSettings(row.id, function (settings) {
        if ((!settings) || (settings.length == 0)) {
            container.innerHTML = "No settings for this plugin";
        }
        else {
            for (var i = 0; i < settings.length; i++) {
                addSettingElement(container, settings[i], null, function (key, value) {
                    var settings = {};
                    settings[key] = value;
                    qdl.setServicePluginSettings(currentServicePlugin, settings);
                });
            }
        }
    });
}

function showRecaptchaSettingsTab() {
    if (currentSettingsTab != "recaptchaSettings") {
        if (currentSettingsTab) {
            document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
            document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        }
        
        document.getElementById("recaptchaSettingsButton").setAttribute("data-current", "true");
        document.getElementById("recaptchaSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "recaptchaSettings";
        var table = document.getElementById("recaptchaTable");

        if (table.childNodes.length == 0) {
            qdl.getRecaptchaPlugins(function (plugins) {                
                for (var i = 0; i < plugins.length; i++) {
                    insertRecaptchaPlugin(plugins[i]);
                }
            });
        }
    }
}

function insertRecaptchaPlugin(plugin, before) {
    var table = document.getElementById("recaptchaTable");
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", plugin.id);
    row.setAttribute("title", plugin.displayName);
    row.setAttribute("data-current", "false");
    row.onclick = function () { setCurrentRecaptchaPlugin(row); }
    var icon = document.createElement("img");
    icon.setAttribute("class", "PluginIcon");
    icon.src = plugin.iconFilePath;
    row.appendChild(icon);
    var name = document.createElement("div");
    name.setAttribute("class", "NoWrap");
    name.innerHTML = plugin.displayName;
    row.appendChild(icon);
    row.appendChild(name);
    table.insertBefore(row, before);
    return row;
}

function setCurrentRecaptchaPlugin(row) {
    if (row.id == currentRecaptchaPlugin) {
        return;
    }
    
    if (currentRecaptchaPlugin) {
        var current = document.getElementById(currentRecaptchaPlugin);
        
        if (current) {
            current.setAttribute("data-current", "false");
        }
    }

    row.setAttribute("data-current", "true");
    currentRecaptchaPlugin = row.id;
    var container = document.getElementById("recaptchaSettings");
    
    for (var i = container.childNodes.length - 1; i >= 0; i--) {
        container.removeChild(container.childNodes[i]);
    }
    
    qdl.getRecaptchaPluginSettings(row.id, function (settings) {
        if ((!settings) || (settings.length == 0)) {
            container.innerHTML = "No settings for this plugin";
        }
        else {
            for (var i = 0; i < settings.length; i++) {
                addSettingElement(container, settings[i], null, function (key, value) {
                    var settings = {};
                    settings[key] = value;
                    qdl.setRecaptchaPluginSettings(currentRecaptchaPlugin, settings);
                });
            }
        }
    });
}

function showDecaptchaSettingsTab() {
    if (currentSettingsTab != "decaptchaSettings") {
        if (currentSettingsTab) {
            document.getElementById(currentSettingsTab + "Button").setAttribute("data-current", "false");
            document.getElementById(currentSettingsTab + "Tab").setAttribute("data-current", "false");
        }
        
        document.getElementById("decaptchaSettingsButton").setAttribute("data-current", "true");
        document.getElementById("decaptchaSettingsTab").setAttribute("data-current", "true");
        currentSettingsTab = "decaptchaSettings";
        var table = document.getElementById("decaptchaTable");

        if (table.childNodes.length == 0) {
            qdl.getDecaptchaPlugins(function (plugins) {
                for (var i = 0; i < plugins.length; i++) {
                    insertDecaptchaPlugin(plugins[i]);
                }
            });
        }
    }
}

function insertDecaptchaPlugin(plugin, before) {
    var table = document.getElementById("decaptchaTable");
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", plugin.id);
    row.setAttribute("title", plugin.displayName);
    row.setAttribute("data-current", "false");
    row.onclick = function () { setCurrentDecaptchaPlugin(row); }
    var icon = document.createElement("img");
    icon.setAttribute("class", "PluginIcon");
    icon.src = plugin.iconFilePath;
    row.appendChild(icon);
    var name = document.createElement("div");
    name.setAttribute("class", "NoWrap");
    name.innerHTML = plugin.displayName;
    row.appendChild(icon);
    row.appendChild(name);
    table.insertBefore(row, before);
    return row;
}

function setCurrentDecaptchaPlugin(row) {
    if (row.id == currentDecaptchaPlugin) {
        return;
    }
    
    if (currentDecaptchaPlugin) {
        var current = document.getElementById(currentDecaptchaPlugin);
        
        if (current) {
            current.setAttribute("data-current", "false");
        }
    }

    row.setAttribute("data-current", "true");
    currentDecaptchaPlugin = row.id;
    var container = document.getElementById("decaptchaSettings");
    
    for (var i = container.childNodes.length - 1; i >= 3; i--) {
        container.removeChild(container.childNodes[i]);
    }
    
    qdl.getSettings(["decaptchaPlugin"], function (settings) {
        document.getElementById("decaptchaCheckBox").checked = settings.decaptchaPlugin == row.id;
    });
    
    qdl.getDecaptchaPluginSettings(row.id, function (settings) {
        if ((!settings) || (settings.length == 0)) {
            container.innerHTML = "No settings for this plugin";
        }
        else {
            for (var i = 0; i < settings.length; i++) {
                addSettingElement(container, settings[i], null, function (key, value) {
                    var settings = {};
                    settings[key] = value;
                    qdl.setDecaptchaPluginSettings(currentDecaptchaPlugin, settings);
                });
            }
        }
    });
}
