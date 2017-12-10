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

#include "javascriptpluginglobalobject.h"
#include "javascriptcaptchatype.h"
#include "javascriptdecaptchaplugin.h"
#include "javascriptnetworkrequest.h"
#include "javascriptrecaptchaplugin.h"
#include "javascriptsearchplugin.h"
#include "javascriptsearchresult.h"
#include "javascriptserviceplugin.h"
#include "javascripturlresult.h"
#include "logger.h"
#include "xmlhttprequest.h"
#include <QNetworkAccessManager>
#include <QRegExp>
#include <QScriptValueIterator>
#include <QTimerEvent>
#if QT_VERSION < 0x050000
#include <QTextDocument>
#endif

JavaScriptPluginGlobalObject::JavaScriptPluginGlobalObject(QScriptEngine *engine) :
    QObject(engine),
    m_engine(engine),
    m_nam(0)
{
    QScriptValue oldGlobal = engine->globalObject();
    QScriptValue thisGlobal = engine->newQObject(this, QScriptEngine::QtOwnership,
            QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater);

    thisGlobal.setProperty("DecaptchaPlugin",
                engine->newQMetaObject(&JavaScriptDecaptchaPluginSignaller::staticMetaObject,
                engine->newFunction(newDecaptchaPlugin)));
    thisGlobal.setProperty("RecaptchaPlugin",
                engine->newQMetaObject(&JavaScriptRecaptchaPluginSignaller::staticMetaObject,
                engine->newFunction(newRecaptchaPlugin)));
    thisGlobal.setProperty("SearchPlugin",
                engine->newQMetaObject(&JavaScriptSearchPluginSignaller::staticMetaObject,
                engine->newFunction(newSearchPlugin)));
    thisGlobal.setProperty("ServicePlugin",
                engine->newQMetaObject(&JavaScriptServicePluginSignaller::staticMetaObject,
                engine->newFunction(newServicePlugin)));
    thisGlobal.setProperty("CaptchaType", engine->newQMetaObject(&JavaScriptCaptchaType::staticMetaObject));
    QScriptValue request = engine->newQObject(new JavaScriptNetworkRequest(engine));
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest>(), request);
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), request);
    thisGlobal.setProperty("NetworkRequest", engine->newFunction(newNetworkRequest));
    QScriptValue sr = engine->newQObject(new JavaScriptSearchResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<SearchResult>(), sr);
    engine->setDefaultPrototype(qMetaTypeId<SearchResult*>(), sr);
    thisGlobal.setProperty("SearchResult", engine->newFunction(newSearchResult));
    qScriptRegisterSequenceMetaType<SearchResultList>(engine);
    QScriptValue ur = engine->newQObject(new JavaScriptUrlResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<UrlResult>(), ur);
    engine->setDefaultPrototype(qMetaTypeId<UrlResult*>(), ur);
    thisGlobal.setProperty("UrlResult", engine->newFunction(newUrlResult));
    qScriptRegisterSequenceMetaType<UrlResultList>(engine);
    thisGlobal.setProperty("XMLHttpRequest", engine->newQMetaObject(&XMLHttpRequest::staticMetaObject,
                engine->newFunction(newXMLHttpRequest)));
    
    QScriptValueIterator iterator(oldGlobal);

    while (iterator.hasNext()) {
        iterator.next();
        thisGlobal.setProperty(iterator.name(), iterator.value());
    }

    engine->setGlobalObject(thisGlobal);
}

QScriptValue JavaScriptPluginGlobalObject::newDecaptchaPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptDecaptchaPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newRecaptchaPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptRecaptchaPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newSearchPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptSearchPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newServicePlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptServicePluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newNetworkRequest(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(QNetworkRequest());
        case 1:
            return engine->toScriptValue(QNetworkRequest(context->argument(0).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                    QObject::tr("NetworkRequest constructor requires either 0 or 1 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newSearchResult(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(SearchResult());
        case 3:
            return engine->toScriptValue(SearchResult(context->argument(0).toString(), context->argument(1).toString(),
                                                      context->argument(2).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                                       QObject::tr("SearchResult constructor requires either 0 or 3 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newUrlResult(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(UrlResult());
        case 2:
            return engine->toScriptValue(UrlResult(context->argument(0).toString(), context->argument(1).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                    QObject::tr("UrlResult constructor requires either 0 or 2 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newXMLHttpRequest(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        XMLHttpRequest *request;
        
        if (JavaScriptPluginGlobalObject *obj =
            qobject_cast<JavaScriptPluginGlobalObject*>(engine->globalObject().toQObject())) {
            request = new XMLHttpRequest(obj->networkAccessManager());
        }
        else {
            request = new XMLHttpRequest;
        }
        
        return engine->newQObject(request, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QNetworkAccessManager* JavaScriptPluginGlobalObject::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

QString JavaScriptPluginGlobalObject::atob(const QString &ascii) const {
    return QString::fromUtf8(QByteArray::fromBase64(ascii.toUtf8()));
}

QString JavaScriptPluginGlobalObject::btoa(const QString &binary) const {
    return QString::fromUtf8(binary.toUtf8().toBase64());
}

QString JavaScriptPluginGlobalObject::decodeHtml(const QString &text) const {
    QString t(text);
    t.replace("&nbsp;", "&#160;");
    t.replace("&iexcl;", "&#161;");
    t.replace("&cent;", "&#162;");
    t.replace("&pound;", "&#163;");
    t.replace("&curren;", "&#164;");
    t.replace("&yen;", "&#165;");
    t.replace("&brvbar;", "&#166;");
    t.replace("&sect;", "&#167;");
    t.replace("&uml;", "&#168;");
    t.replace("&copy;", "&#169;");
    t.replace("&ordf;", "&#170;");
    t.replace("&laquo;", "&#171;");
    t.replace("&not;", "&#172;");
    t.replace("&shy;", "&#173;");
    t.replace("&reg;", "&#174;");
    t.replace("&macr;", "&#175;");
    t.replace("&deg;", "&#176;");
    t.replace("&plusmn;", "&#177;");
    t.replace("&sup2;", "&#178;");
    t.replace("&sup3;", "&#179;");
    t.replace("&acute;", "&#180;");
    t.replace("&micro;", "&#181;");
    t.replace("&para;", "&#182;");
    t.replace("&middot;", "&#183;");
    t.replace("&cedil;", "&#184;");
    t.replace("&sup1;", "&#185;");
    t.replace("&ordm;", "&#186;");
    t.replace("&raquo;", "&#187;");
    t.replace("&frac14;", "&#188;");
    t.replace("&frac12;", "&#189;");
    t.replace("&frac34;", "&#190;");
    t.replace("&iquest;", "&#191;");
    t.replace("&Agrave;", "&#192;");
    t.replace("&Aacute;", "&#193;");
    t.replace("&Acirc;", "&#194;");
    t.replace("&Atilde;", "&#195;");
    t.replace("&Auml;", "&#196;");
    t.replace("&Aring;", "&#197;");
    t.replace("&AElig;", "&#198;");
    t.replace("&Ccedil;", "&#199;");
    t.replace("&Egrave;", "&#200;");
    t.replace("&Eacute;", "&#201;");
    t.replace("&Ecirc;", "&#202;");
    t.replace("&Euml;", "&#203;");
    t.replace("&Igrave;", "&#204;");
    t.replace("&Iacute;", "&#205;");
    t.replace("&Icirc;", "&#206;");
    t.replace("&Iuml;", "&#207;");
    t.replace("&ETH;", "&#208;");
    t.replace("&Ntilde;", "&#209;");
    t.replace("&Ograve;", "&#210;");
    t.replace("&Oacute;", "&#211;");
    t.replace("&Ocirc;", "&#212;");
    t.replace("&Otilde;", "&#213;");
    t.replace("&Ouml;", "&#214;");
    t.replace("&times;", "&#215;");
    t.replace("&Oslash;", "&#216;");
    t.replace("&Ugrave;", "&#217;");
    t.replace("&Uacute;", "&#218;");
    t.replace("&Ucirc;", "&#219;");
    t.replace("&Uuml;", "&#220;");
    t.replace("&Yacute;", "&#221;");
    t.replace("&THORN;", "&#222;");
    t.replace("&szlig;", "&#223;");
    t.replace("&agrave;", "&#224;");
    t.replace("&aacute;", "&#225;");
    t.replace("&acirc;", "&#226;");
    t.replace("&atilde;", "&#227;");
    t.replace("&auml;", "&#228;");
    t.replace("&aring;", "&#229;");
    t.replace("&aelig;", "&#230;");
    t.replace("&ccedil;", "&#231;");
    t.replace("&egrave;", "&#232;");
    t.replace("&eacute;", "&#233;");
    t.replace("&ecirc;", "&#234;");
    t.replace("&euml;", "&#235;");
    t.replace("&igrave;", "&#236;");
    t.replace("&iacute;", "&#237;");
    t.replace("&icirc;", "&#238;");
    t.replace("&iuml;", "&#239;");
    t.replace("&eth;", "&#240;");
    t.replace("&ntilde;", "&#241;");
    t.replace("&ograve;", "&#242;");
    t.replace("&oacute;", "&#243;");
    t.replace("&ocirc;", "&#244;");
    t.replace("&otilde;", "&#245;");
    t.replace("&ouml;", "&#246;");
    t.replace("&divide;", "&#247;");
    t.replace("&oslash;", "&#248;");
    t.replace("&ugrave;", "&#249;");
    t.replace("&uacute;", "&#250;");
    t.replace("&ucirc;", "&#251;");
    t.replace("&uuml;", "&#252;");
    t.replace("&yacute;", "&#253;");
    t.replace("&thorn;", "&#254;");
    t.replace("&yuml;", "&#255;");
    t.replace("&quot;", "&#34;");
    t.replace("&amp;", "&#38;");
    t.replace("&lt;", "&#60;");
    t.replace("&gt;", "&#62;");
    t.replace("&OElig;", "&#338;");
    t.replace("&oelig;", "&#339;");
    t.replace("&Scaron;", "&#352;");
    t.replace("&scaron;", "&#353;");
    t.replace("&Yuml;", "&#376;");
    t.replace("&circ;", "&#710;");
    t.replace("&tilde;", "&#732;");
    t.replace("&ensp;", "&#8194;");
    t.replace("&emsp;", "&#8195;");
    t.replace("&thinsp;", "&#8201;");
    t.replace("&zwnj;", "&#8204;");
    t.replace("&zwj;", "&#8205;");
    t.replace("&lrm;", "&#8206;");
    t.replace("&rlm;", "&#8207;");
    t.replace("&ndash;", "&#8211;");
    t.replace("&mdash;", "&#8212;");
    t.replace("&lsquo;", "&#8216;");
    t.replace("&rsquo;", "&#8217;");
    t.replace("&sbquo;", "&#8218;");
    t.replace("&ldquo;", "&#8220;");
    t.replace("&rdquo;", "&#8221;");
    t.replace("&bdquo;", "&#8222;");
    t.replace("&dagger;", "&#8224;");
    t.replace("&Dagger;", "&#8225;");
    t.replace("&permil;", "&#8240;");
    t.replace("&lsaquo;", "&#8249;");
    t.replace("&rsaquo;", "&#8250;");
    t.replace("&euro;", "&#8364;");
    t.replace("&fnof;", "&#402;");
    t.replace("&Alpha;", "&#913;");
    t.replace("&Beta;", "&#914;");
    t.replace("&Gamma;", "&#915;");
    t.replace("&Delta;", "&#916;");
    t.replace("&Epsilon;", "&#917;");
    t.replace("&Zeta;", "&#918;");
    t.replace("&Eta;", "&#919;");
    t.replace("&Theta;", "&#920;");
    t.replace("&Iota;", "&#921;");
    t.replace("&Kappa;", "&#922;");
    t.replace("&Lambda;", "&#923;");
    t.replace("&Mu;", "&#924;");
    t.replace("&Nu;", "&#925;");
    t.replace("&Xi;", "&#926;");
    t.replace("&Omicron;", "&#927;");
    t.replace("&Pi;", "&#928;");
    t.replace("&Rho;", "&#929;");
    t.replace("&Sigma;", "&#931;");
    t.replace("&Tau;", "&#932;");
    t.replace("&Upsilon;", "&#933;");
    t.replace("&Phi;", "&#934;");
    t.replace("&Chi;", "&#935;");
    t.replace("&Psi;", "&#936;");
    t.replace("&Omega;", "&#937;");
    t.replace("&alpha;", "&#945;");
    t.replace("&beta;", "&#946;");
    t.replace("&gamma;", "&#947;");
    t.replace("&delta;", "&#948;");
    t.replace("&epsilon;", "&#949;");
    t.replace("&zeta;", "&#950;");
    t.replace("&eta;", "&#951;");
    t.replace("&theta;", "&#952;");
    t.replace("&iota;", "&#953;");
    t.replace("&kappa;", "&#954;");
    t.replace("&lambda;", "&#955;");
    t.replace("&mu;", "&#956;");
    t.replace("&nu;", "&#957;");
    t.replace("&xi;", "&#958;");
    t.replace("&omicron;", "&#959;");
    t.replace("&pi;", "&#960;");
    t.replace("&rho;", "&#961;");
    t.replace("&sigmaf;", "&#962;");
    t.replace("&sigma;", "&#963;");
    t.replace("&tau;", "&#964;");
    t.replace("&upsilon;", "&#965;");
    t.replace("&phi;", "&#966;");
    t.replace("&chi;", "&#967;");
    t.replace("&psi;", "&#968;");
    t.replace("&omega;", "&#969;");
    t.replace("&thetasym;", "&#977;");
    t.replace("&upsih;", "&#978;");
    t.replace("&piv;", "&#982;");
    t.replace("&bull;", "&#8226;");
    t.replace("&hellip;", "&#8230;");
    t.replace("&prime;", "&#8242;");
    t.replace("&Prime;", "&#8243;");
    t.replace("&oline;", "&#8254;");
    t.replace("&frasl;", "&#8260;");
    t.replace("&weierp;", "&#8472;");
    t.replace("&image;", "&#8465;");
    t.replace("&real;", "&#8476;");
    t.replace("&trade;", "&#8482;");
    t.replace("&alefsym;", "&#8501;");
    t.replace("&larr;", "&#8592;");
    t.replace("&uarr;", "&#8593;");
    t.replace("&rarr;", "&#8594;");
    t.replace("&darr;", "&#8595;");
    t.replace("&harr;", "&#8596;");
    t.replace("&crarr;", "&#8629;");
    t.replace("&lArr;", "&#8656;");
    t.replace("&uArr;", "&#8657;");
    t.replace("&rArr;", "&#8658;");
    t.replace("&dArr;", "&#8659;");
    t.replace("&hArr;", "&#8660;");
    t.replace("&forall;", "&#8704;");
    t.replace("&part;", "&#8706;");
    t.replace("&exist;", "&#8707;");
    t.replace("&empty;", "&#8709;");
    t.replace("&nabla;", "&#8711;");
    t.replace("&isin;", "&#8712;");
    t.replace("&notin;", "&#8713;");
    t.replace("&ni;", "&#8715;");
    t.replace("&prod;", "&#8719;");
    t.replace("&sum;", "&#8721;");
    t.replace("&minus;", "&#8722;");
    t.replace("&lowast;", "&#8727;");
    t.replace("&radic;", "&#8730;");
    t.replace("&prop;", "&#8733;");
    t.replace("&infin;", "&#8734;");
    t.replace("&ang;", "&#8736;");
    t.replace("&and;", "&#8743;");
    t.replace("&or;", "&#8744;");
    t.replace("&cap;", "&#8745;");
    t.replace("&cup;", "&#8746;");
    t.replace("&int;", "&#8747;");
    t.replace("&there4;", "&#8756;");
    t.replace("&sim;", "&#8764;");
    t.replace("&cong;", "&#8773;");
    t.replace("&asymp;", "&#8776;");
    t.replace("&ne;", "&#8800;");
    t.replace("&equiv;", "&#8801;");
    t.replace("&le;", "&#8804;");
    t.replace("&ge;", "&#8805;");
    t.replace("&sub;", "&#8834;");
    t.replace("&sup;", "&#8835;");
    t.replace("&nsub;", "&#8836;");
    t.replace("&sube;", "&#8838;");
    t.replace("&supe;", "&#8839;");
    t.replace("&oplus;", "&#8853;");
    t.replace("&otimes;", "&#8855;");
    t.replace("&perp;", "&#8869;");
    t.replace("&sdot;", "&#8901;");
    t.replace("&lceil;", "&#8968;");
    t.replace("&rceil;", "&#8969;");
    t.replace("&lfloor;", "&#8970;");
    t.replace("&rfloor;", "&#8971;");
    t.replace("&lang;", "&#9001;");
    t.replace("&rang;", "&#9002;");
    t.replace("&loz;", "&#9674;");
    t.replace("&spades;", "&#9824;");
    t.replace("&clubs;", "&#9827;");
    t.replace("&hearts;", "&#9829;");
    t.replace("&diams;", "&#9830;");

    QRegExp re("&#(\\d+);");
    int i;

    while ((i = re.indexIn(t) != -1)) {
        t.replace(re.cap(), QChar(re.cap(1).toInt()));
        ++i;
    }

    return t;
}

QString JavaScriptPluginGlobalObject::encodeHtml(const QString &text) const {
#if QT_VERSION >= 0x050000
    return text.toHtmlEscaped();
#else
    return Qt::escape(text);
#endif
}

void JavaScriptPluginGlobalObject::clearInterval(int timerId) {
    if (m_intervals.contains(timerId)) {
        m_intervals.remove(timerId);
        killTimer(timerId);
    }
}

void JavaScriptPluginGlobalObject::clearTimeout(int timerId) {
    if (m_timeouts.contains(timerId)) {
        m_timeouts.remove(timerId);
        killTimer(timerId);
    }
}

int JavaScriptPluginGlobalObject::setInterval(const QScriptValue &function, int msecs) {
    if ((function.isFunction()) || (function.isString())) {
        const int timerId = startTimer(msecs);
        m_intervals[timerId] = function;
        return timerId;
    }

    return -1;
}

int JavaScriptPluginGlobalObject::setTimeout(const QScriptValue &function, int msecs) {
    if ((function.isFunction()) || (function.isString())) {
        const int timerId = startTimer(msecs);
        m_timeouts[timerId] = function;
        return timerId;
    }

    return -1;
}

bool JavaScriptPluginGlobalObject::callFunction(QScriptValue function) const {
    if (function.isFunction()) {
        const QScriptValue result = function.call(QScriptValue());

        if (result.isError()) {
            Logger::log("JavaScriptPluginGlobalObject::callFunction(). Error: " + result.toString());
            return false;
        }

        return true;
    }

    if (!m_engine.isNull()) {
        const QScriptValue result = m_engine->globalObject().property(function.toString()).call(QScriptValue());

        if (result.isError()) {
            Logger::log("JavaScriptPluginGlobalObject::callFunction(). Error: " + result.toString());
            return false;
        }

        return true;
    }
    
    return false;
}

void JavaScriptPluginGlobalObject::timerEvent(QTimerEvent *event) {
    if (m_intervals.contains(event->timerId())) {
        if (!callFunction(m_intervals.value(event->timerId()))) {
            clearInterval(event->timerId());
        }
        
        event->accept();
        return;
    }

    if (m_timeouts.contains(event->timerId())) {
        callFunction(m_timeouts.value(event->timerId()));
        clearTimeout(event->timerId());
        event->accept();
    }
}
