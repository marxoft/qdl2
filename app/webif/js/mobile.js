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
    
    loadTransfers(0, -1);
}

function loadTransfers(offset, limit) {
    qdl.getTransfers(offset, limit, false, function (result) {
        document.getElementById("transfersTitle").innerHTML = result.active + "DLs - " + result.speedString; 
        var transfers = result.transfers;
        
        for (var i = 0; i < transfers.length; i++) {
            insertTransfer(transfers[i]);
        }
        
        //checkForCaptchaAndSettingsRequest();
    });
}

function showChildTransfers(item) {
    item.setAttribute("data-expanded", "true");
    item.childNodes[0].innerHTML = "-";
    qdl.getTransfer(item.id, true, function (transfer) {
        for (var i = 0; i < transfer.children.length; i++) {
            item = insertTransfer(transfer.children[i], item.nextSibling);
        }
    });
}

function hideChildTransfers(item) {
    item.setAttribute("data-expanded", "false");
    item.childNodes[0].innerHTML = "+";    
    var next = item.nextSibling;
        
    while ((next) && (next.getAttribute("data-itemtype") == TransferItemType.TransferType)) {        
        item.parentNode.removeChild(next);
        next = item.nextSibling;
    }
}

function toggleItemExpanded(item) {
    if (item.getAttribute("data-expanded") == "true") {
        hideChildTransfers(item);
    }
    else {
        showChildTransfers(item);
    }
}

function insertTransfer(transfer, before) {
    var list = document.getElementById("transfersList");
    var item = document.createElement("li");
    item.setAttribute("class", "ListItem");
    item.setAttribute("id", transfer.id);
    item.setAttribute("title", transfer.name);
    item.setAttribute("data-cancancel", transfer.canCancel);
    item.setAttribute("data-canpause", transfer.canPause);
    item.setAttribute("data-canstart", transfer.canStart);
    item.setAttribute("data-current", "false");
    item.setAttribute("data-expanded", "false");
    item.setAttribute("data-itemtype", transfer.itemType);
    item.setAttribute("data-priority", transfer.priority);
    item.setAttribute("data-status", transfer.status);
    item.onclick = function (event) { setCurrentTransfer(item); showMenu(event, item); }
    
    var content = document.createElement("div");
    content.setAttribute("class", "ListItemContent");
    
    if (transfer.itemType == TransferItemType.PackageType) {
        var indicator = document.createElement("div");
        indicator.setAttribute("class", "Indicator");
        indicator.innerHTML = "+";
        indicator.onclick = function (event) { event.stopPropagation(); toggleItemExpanded(item); }
        content.appendChild(indicator);
        
        var text = document.createElement("div");
        text.setAttribute("class", "ListItemText");
        text.innerHTML = transfer.name + "<br>" +
            (transfer.status == TransferItemStatus.Null ? transfer.progressString : transfer.statusString);
        content.appendChild(text);
    }
    else {
        var icon = document.createElement("img");
        icon.setAttribute("class", "TransferIcon");
        icon.src = transfer.pluginIconPath;
        content.appendChild(icon);
        
        var text = document.createElement("div");
        text.setAttribute("class", "ListItemText");
        text.innerHTML = transfer.name + "<br>" + transfer.statusString;
        content.appendChild(text);
        
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
        content.appendChild(progressBar);
    }
    
    item.appendChild(content);
    
    list.insertBefore(item, before);
    return item;
}

function removeTransfer(item) {
    if (item.getAttribute("data-expanded") == "true") {
        hideChildTransfers(item);
    }
    
    item.parentNode.removeChild(item);
}

function updateTransfer(transfer) {
    var item = document.getElementById(transfer.id);
        
    if (item) {
        item.setAttribute("title", transfer.name);
        item.setAttribute("data-priority", transfer.priority);
        item.setAttribute("data-status", transfer.status);
        
        if (transfer.itemType == TransferItemType.PackageType) {
            item.childNodes[0].childNodes[1].innerHTML = transfer.name + "<br>" +
                (transfer.status == TransferItemStatus.Null ? transfer.progressString : transfer.statusString);
            
            var children = transfer.children;
            
            if (children) {
                for (var i = 0; i < children.length; i++) {
                    updateTransfer(children[i]);
                }
            }
        }
        else {
            item.childNodes[0].childNodes[1].innerHTML = transfer.name + "<br>" + transfer.statusString;
            var progressBar = item.childNodes[2].childNodes[0];
            progressBar.childNodes[0].innerHTML = transfer.progressString;
            progressBar.childNodes[1].style.width = transfer.progress + "%";
        }
    }
}

function setCurrentTransfer(item) {
    if (item.id == currentTransfer) {
        return;
    }
    
    if (currentTransfer) {
        var current = document.getElementById(currentTransfer);
        
        if (current) {
            current.setAttribute("data-current", "false");
        }
    }

    item.setAttribute("data-current", "true");
    currentTransfer = item.id;
}

function showMenu(event, item) {
    if (item.getAttribute("data-itemtype") == TransferItemType.PackageType) {
        showPackageMenu(event, item);
    }
    else {
        showTransferMenu(event, item);
    }
}

function showPackageMenu(event, item) {
    var background = document.getElementById("transfersPopupBackground");
    background.style.display = "block"
    background.onclick = cancelPackageMenu;
    document.getElementById("packageStartItem").style.display =
        (item.getAttribute("data-canstart") == "true" ? "block" : "none");
    document.getElementById("packagePauseItem").style.display =
        (item.getAttribute("data-canpause") == "true" ? "block" : "none");
    document.getElementById("packageCancelItem").style.display =
        document.getElementById("packageCancelDeleteItem").style.display =
        (item.getAttribute("data-cancancel") == "true" ? "block" : "none");
    var menu = document.getElementById("packageMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelPackageMenu() {
    document.getElementById("packageMenu").style.display = "none";
    var background = document.getElementById("transfersPopupBackground");
    background.style.display = "none"
    background.onclick = null;
}

function showTransferMenu(event, item) {
    var background = document.getElementById("transfersPopupBackground");
    background.style.display = "block"
    background.onclick = cancelTransferMenu;
    document.getElementById("transferStartItem").style.display =
        (item.getAttribute("data-canstart") == "true" ? "block" : "none");
    document.getElementById("transferPauseItem").style.display =
        (item.getAttribute("data-canpause") == "true" ? "block" : "none");
    document.getElementById("transferCancelItem").style.display =
        document.getElementById("transferCancelDeleteItem").style.display =
        (item.getAttribute("data-cancancel") == "true" ? "block" : "none");
    var menu = document.getElementById("transferMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelTransferMenu() {
    document.getElementById("transferMenu").style.display = "none";
    var background = document.getElementById("transfersPopupBackground");
    background.style.display = "none"
    background.onclick = null;
}
