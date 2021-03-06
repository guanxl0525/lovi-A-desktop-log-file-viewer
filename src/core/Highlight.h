/*
 * Copyright 2019 Aurélien Gâteau <mail@agateau.com>
 *
 * This file is part of Lovi.
 *
 * Lovi is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "Color.h"
#include "Conditions.h"

#include <QColor>
#include <QString>

#include <memory>

class LogFormat;

class Highlight {
public:
    enum Scope { Cell, Row };
    explicit Highlight(LogFormat* logFormat);

    void setConditionDefinition(const QString& definition);
    QString conditionDefinition() const;
    void updateCondition();

    void setScope(Scope scope);
    Scope scope() const {
        return mScope;
    }

    Condition* condition() const {
        return mCondition.get();
    }

    void setBgColor(const OptionalColor& color);

    OptionalColor bgColor() const {
        return mBgColor;
    }

    void setFgColor(const OptionalColor& color);

    OptionalColor fgColor() const {
        return mFgColor;
    }

    LogFormat* logFormat() const {
        return mLogFormat;
    }

private:
    LogFormat* const mLogFormat;

    QString mConditionDefinition;

    std::unique_ptr<Condition> mCondition;

    Scope mScope = Cell;

    OptionalColor mBgColor;
    OptionalColor mFgColor;
};

#endif // HIGHLIGHT_H
