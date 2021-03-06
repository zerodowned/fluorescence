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


#ifndef FLUO_UI_PYTHON_SCRIPTLOADER_HPP
#define FLUO_UI_PYTHON_SCRIPTLOADER_HPP

#include <boost/python.hpp>
#include <boost/filesystem/path.hpp>
#include <map>
#include <list>

#include <misc/string.hpp>
#include <misc/config.hpp>

namespace fluo {
namespace ui {

class GumpMenu;

namespace python {

class ScriptLoader {
public:
    ScriptLoader();
    ~ScriptLoader();

    void init();
    void setThemePath(const boost::filesystem::path& themePath);
    void setShardConfig(Config& config);

    void requestReload();

    // returns true if the scripts were reloaded
    bool step(unsigned int elapsedMillis);

    void logError() const;

    void openGump(const UnicodeString& name);
    void openGump(const UnicodeString& name, boost::python::dict& args);

    boost::python::object loadModule(const UnicodeString& name);

    void saveToFile(const boost::filesystem::path& path);
    void loadFromFile(const boost::filesystem::path& path);

    boost::python::object callFunction(GumpMenu* menu, const UnicodeString& moduleName, const char* functionName);

private:
    bool initialized_;

    std::map<UnicodeString, boost::python::object> pythonModules_;

    bool shouldReload_;
    void unload();
    void reload();

    void saveGumps(std::list<std::pair<UnicodeString, boost::python::object> >& savedObjects);
    void loadGumps(std::list<std::pair<UnicodeString, boost::python::object> >& savedObjects);
};

}
}
}

#endif

