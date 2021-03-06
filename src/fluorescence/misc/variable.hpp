/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FLUO_MISC_VARIABLE_HPP
#define FLUO_MISC_VARIABLE_HPP

#include <vector>
#include <boost/filesystem/path.hpp>
#include <misc/string.hpp>

namespace fluo {

class Variable {
public:
    class BadCastException : public std::exception {
    public:
        const char* what() const throw() {
            return "Bad config value cast";
        }
    };

    static const unsigned int TYPE_UNKNOWN = 0;
    static const unsigned int TYPE_INT = 1;
    static const unsigned int TYPE_BOOL = 2;
    static const unsigned int TYPE_STRING = 3;
    static const unsigned int TYPE_PATH = 4;

    Variable();

    bool isInt() const;
    bool isBool() const;
    bool isString() const;
    bool isPath() const;

    bool isDefault() const;

    bool parse(const char* str);

    const bool asBool() const;
    const int asInt() const;
    const UnicodeString& asString() const;
    std::string asUtf8() const;
    const boost::filesystem::path& asPath() const;

    void setBool(bool val, bool isDefault = false);
    void setInt(int val, bool isDefault = false);
    void setString(const UnicodeString& val, bool isDefault = false);
    void setPath(const boost::filesystem::path& val, bool isDefault = false);

    unsigned int valueType() const;

    void toIntList(std::vector<int>& vec) const;

private:
    bool isDefault_;
    unsigned int valueType_;

    int valueInt_;
    bool valueBool_;
    UnicodeString valueString_;
    boost::filesystem::path valuePath_;
};

}

#endif
