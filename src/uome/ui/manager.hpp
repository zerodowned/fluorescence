#ifndef UOME_GUI_MANAGER_HPP
#define UOME_GUI_MANAGER_HPP

#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/gui.h>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <list>

#include <misc/config.hpp>

namespace uome {
namespace ui {

class IngameView;
class RenderQueue;
class CursorManager;
class DoubleClickHandler;
class GumpMenu;

class Manager {
public:
    static bool create();
    static void destroy();
    static Manager* getSingleton();

    bool setShardConfig(Config& config);

    static CL_GraphicContext& getGraphicsContext();
    static boost::shared_ptr<CL_DisplayWindow> getMainWindow();

    static CL_Texture* provideTexture(unsigned int width, unsigned int height);

    static boost::shared_ptr<RenderQueue> getRenderQueue();
    static boost::shared_ptr<CL_GUIManager> getGuiManager();
    static boost::shared_ptr<CursorManager> getCursorManager();
    static boost::shared_ptr<DoubleClickHandler> getDoubleClickHandler();

    void step();

    GumpMenu* openXmlGump(const std::string& name);
    void closeGumpMenu(const std::string& name);
    void closeGumpMenu(GumpMenu* menu);


    // special gumps
    bool openMessageBox(const std::string& message);

private:
    static Manager* singleton_;

    Manager();
    ~Manager();
    Manager(const Manager& copy) { }
    void operator=(const Manager& copy) { }

    boost::shared_ptr<RenderQueue> renderQueue_;

    // clan lib setup stuff
    CL_SetupCore clSetupCore;
    CL_SetupDisplay clSetupDisplay;
    CL_SetupGL clSetupGL;
    CL_SetupGUI clSetupGUI;

    // clan lib gui stuff
    boost::shared_ptr<CL_GUIManager> guiManager_;
    boost::shared_ptr<CL_GUIWindowManagerTexture> windowManager_;
    boost::shared_ptr<CL_DisplayWindow> mainWindow_;

    boost::shared_ptr<CursorManager> cursorManager_;

    boost::shared_ptr<DoubleClickHandler> doubleClickHandler_;

    std::list<GumpMenu*> closeList_;
    void processCloseList();

    CL_Slot slotCloseWindow;

    void loadFontDirectory(const boost::filesystem::path& path);
};

}
}

#endif
