/*
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

#include "javascriptpluginengine.h"
#include "javascriptpluginglobalobject.h"

JavaScriptPluginEngine* JavaScriptPluginEngine::self = 0;

JavaScriptPluginEngine::JavaScriptPluginEngine() :
    QScriptEngine(),
    m_global(new JavaScriptPluginGlobalObject(this))
{
    installTranslatorFunctions();
}

JavaScriptPluginEngine::~JavaScriptPluginEngine() {
    self = 0;
}

JavaScriptPluginEngine* JavaScriptPluginEngine::instance() {
    return self ? self : self = new JavaScriptPluginEngine;
}
