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

#ifndef JAVASCRIPTCAPTCHATYPE_H
#define JAVASCRIPTCAPTCHATYPE_H

#include "captchatype.h"
#include <QObject>

class JavaScriptCaptchaType : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)

public:
    enum Type {
        Image = CaptchaType::Image,
        NoCaptcha = CaptchaType::NoCaptcha,
        Unknown = CaptchaType::Unknown
    };

private:
    JavaScriptCaptchaType() : QObject() {}
};

#endif // JAVASCRIPTCAPTCHATYPE_H
