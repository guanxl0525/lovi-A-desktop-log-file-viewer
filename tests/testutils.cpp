#include "testutils.h"

std::ostream& operator<<(std::ostream& ostr, const QString& str) {
    ostr << '"' << str.toStdString() << '"';
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const QStringList& lst) {
    ostr << '{';
    for (const auto& str : lst) {
        ostr << '"' << str.toStdString() << "\", ";
    }
    ostr << '}';
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const QUrl& url) {
    ostr << '"' << url.toEncoded().constData() << '"';
    return ostr;
}