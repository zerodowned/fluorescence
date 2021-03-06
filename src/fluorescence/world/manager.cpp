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



#include "manager.hpp"

#include "sectormanager.hpp"
#include "lightmanager.hpp"
#include "smoothmovementmanager.hpp"
#include "mobile.hpp"
#include "dynamicitem.hpp"
#include "overheadmessage.hpp"
#include "playerwalkmanager.hpp"
#include "effect.hpp"
#include "syslog.hpp"
#include "sector.hpp"
#include "weathereffect.hpp"

#include <ui/manager.hpp>
#include <ui/cliprectmanager.hpp>
#include <ui/particles/xmlloader.hpp>

#include <net/manager.hpp>
#include <net/packets/34_statskillquery.hpp>

#include <misc/exception.hpp>
#include <misc/log.hpp>

namespace fluo {
namespace world {

Manager* Manager::singleton_ = NULL;

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw Exception("fluo::world::Manager Singleton not created yet");
    }
    return singleton_;
}

bool Manager::create(const Config& config) {
    if (!singleton_) {
        try {
            singleton_ = new Manager(config);
        } catch (const std::exception& ex) {
            LOG_EMERGENCY << "Error initializing world::Manager: " << ex.what() << std::endl;
            return false;
        }
    }

    return true;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager::Manager(const Config& config) : currentMapId_(0), roofHeight_(INT_MAX), weatherType_(0xFF), lastPlayerX_(-1), lastPlayerY_(-1) {
    sectorManager_.reset(new SectorManager(config));
    lightManager_.reset(new LightManager());
    smoothMovementManager_.reset(new SmoothMovementManager());
    playerWalkManager_.reset(new PlayerWalkManager());

    sysLog_.reset(new SysLog());

    setAutoDeleteRange(18);
}

Manager::~Manager() {
    LOG_INFO << "world::Manager shutdown" << std::endl;
}

boost::shared_ptr<SectorManager> Manager::getSectorManager() {
    return getSingleton()->sectorManager_;
}

boost::shared_ptr<SmoothMovementManager> Manager::getSmoothMovementManager() {
    return getSingleton()->smoothMovementManager_;
}

boost::shared_ptr<PlayerWalkManager> Manager::getPlayerWalkManager() {
    return getSingleton()->playerWalkManager_;
}

boost::shared_ptr<SysLog> Manager::getSysLog() {
    return getSingleton()->sysLog_;
}


unsigned int Manager::getCurrentMapId() {
    return currentMapId_;
}

void Manager::setCurrentMapId(unsigned int id) {
    if (currentMapId_ != id) {
        currentMapId_ = id;
        sectorManager_->onMapChange();

        // force sector update
        lastPlayerX_ = -1;
        lastPlayerY_ = -1;

        if (player_) {
            player_->onLocationChanged(player_->getLocation());
        }
    }
}

boost::shared_ptr<LightManager> Manager::getLightManager() {
    return getSingleton()->lightManager_;
}

boost::shared_ptr<Mobile> Manager::initPlayer(Serial serial) {
    player_.reset(new Mobile(serial));
    mobiles_[serial] = player_;

    net::packets::StatSkillQuery queryPacket(player_->getSerial(), net::packets::StatSkillQuery::QUERY_SKILLS);
    net::Manager::getSingleton()->send(queryPacket);

    return player_;
}

boost::shared_ptr<Mobile> Manager::getPlayer() {
    return player_;
}

void Manager::deleteObject(Serial serial) {
    boost::shared_ptr<Mobile> mob = getMobile(serial, false);
    if (mob) {
        mob->onDelete();
        mobiles_.erase(serial);
    } else {
        boost::shared_ptr<DynamicItem> itm = getDynamicItem(serial, false);
        if (itm) {
            itm->onDelete();
            dynamicItems_.erase(serial);
        }
    }

    smoothMovementManager_->clear(serial);
}

boost::shared_ptr<Mobile> Manager::getMobile(Serial serial, bool createIfNotExists) {
    boost::shared_ptr<Mobile> itm;

    std::map<Serial, boost::shared_ptr<Mobile> >::iterator iter = mobiles_.find(serial);
    if (iter != mobiles_.end()) {
        itm = iter->second;
    } else if (createIfNotExists) {
        itm.reset(new Mobile(serial));
        mobiles_[serial] = itm;
    }

    return itm;
}

boost::shared_ptr<DynamicItem> Manager::getDynamicItem(Serial serial, bool createIfNotExists) {
    boost::shared_ptr<DynamicItem> itm;

    std::map<Serial, boost::shared_ptr<DynamicItem> >::iterator iter = dynamicItems_.find(serial);
    if (iter != dynamicItems_.end()) {
        itm = iter->second;
    } else if(createIfNotExists) {
        itm.reset(new DynamicItem(serial));
        dynamicItems_[serial] = itm;
    }

    return itm;
}

void Manager::step(unsigned int elapsedMillis) {
    sysLog_->update(elapsedMillis);

    update(elapsedMillis);
}

void Manager::update(unsigned int elapsedMillis) {
    smoothMovementManager_->update(elapsedMillis);

    int playerX = player_->getLocXGame();
    int playerY = player_->getLocYGame();

    // player sector changed
    if (playerX / 8 != lastPlayerX_ / 8 ||
            playerY / 8 != lastPlayerY_ / 8) {
        LOG_DEBUG << "Sector change" << std::endl;
        sectorManager_->updateSectorList();
    }
    lastPlayerX_ = playerX;
    lastPlayerY_ = playerY;

    std::list<Serial> outOfRangeDelete;

    std::map<Serial, boost::shared_ptr<Mobile> >::iterator mobIter = mobiles_.begin();
    std::map<Serial, boost::shared_ptr<Mobile> >::iterator mobEnd = mobiles_.end();

    for (; mobIter != mobEnd; ++mobIter) {
        if ((unsigned int)abs((int)mobIter->second->getLocXGame() - playerX) > autoDeleteRange_ ||
                (unsigned int)abs((int)mobIter->second->getLocYGame() - playerY) > autoDeleteRange_) {
            outOfRangeDelete.push_back(mobIter->first);
        } else {
            updateObject(mobIter->second.get(), elapsedMillis);
        }
    }

    std::map<Serial, boost::shared_ptr<DynamicItem> >::iterator itmIter = dynamicItems_.begin();
    std::map<Serial, boost::shared_ptr<DynamicItem> >::iterator itmEnd = dynamicItems_.end();

    for (; itmIter != itmEnd; ++itmIter) {
        if (((unsigned int)(abs((int)itmIter->second->getLocXGame() - playerX)) > autoDeleteRange_ ||
                (unsigned int)abs((int)itmIter->second->getLocYGame() - playerY) > autoDeleteRange_) && !itmIter->second->hasParent()) {
            outOfRangeDelete.push_back(itmIter->first);
        } else {
            updateObject(itmIter->second.get(), elapsedMillis);
        }
    }

    std::list<boost::shared_ptr<OverheadMessage> >::iterator msgIter = overheadMessages_.begin();
    std::list<boost::shared_ptr<OverheadMessage> >::iterator msgEnd = overheadMessages_.end();
    std::list<boost::shared_ptr<OverheadMessage> > expiredMessages;

    for (; msgIter != msgEnd; ++msgIter) {
        updateObject(msgIter->get(), elapsedMillis);

        if ((*msgIter)->isExpired()) {
            expiredMessages.push_back(*msgIter);
        }
    }

    if (!expiredMessages.empty()) {
        msgIter = expiredMessages.begin();
        msgEnd = expiredMessages.end();

        for (; msgIter != msgEnd; ++msgIter) {
            (*msgIter)->repaintRectangle(false);
            (*msgIter)->expire();
        }
    }


    std::list<boost::shared_ptr<Effect> >::iterator effectIter = effects_.begin();
    std::list<boost::shared_ptr<Effect> >::iterator effectEnd = effects_.end();
    std::list<boost::shared_ptr<Effect> > expiredEffects;

    for (; effectIter != effectEnd; ++effectIter) {
        (*effectIter)->update(elapsedMillis); // update effect itself (e.g. position)
        updateObject(effectIter->get(), elapsedMillis);

        if ((*effectIter)->isExpired()) {
            expiredEffects.push_back(*effectIter);
        }
    }

    if (!expiredEffects.empty()) {
        effectIter = expiredEffects.begin();
        effectEnd = expiredEffects.end();

        for (; effectIter != effectEnd; ++effectIter) {
            (*effectIter)->repaintRectangle(true);
            (*effectIter)->onDelete();
            effects_.remove(*effectIter);
        }
    }

    if (weatherEffects_.first) {
        weatherEffects_.first->update(elapsedMillis);
    }
    if (weatherEffects_.second) {
        weatherEffects_.second->update(elapsedMillis);
    }


    if (!outOfRangeDelete.empty()) {
        //LOG_DEBUG << "out of range coords=" << playerX << "/" << playerY << std::endl;
        std::list<Serial>::const_iterator iter = outOfRangeDelete.begin();
        std::list<Serial>::const_iterator end = outOfRangeDelete.end();

        for (; iter != end; ++iter) {
            deleteObject(*iter);
        }
    }

    sectorManager_->update(elapsedMillis);

    // do after sector sorting
    playerWalkManager_->update(elapsedMillis);
}

void Manager::updateObject(IngameObject* obj, unsigned int elapsedMillis) {
    obj->updateRenderData(elapsedMillis);
    bool depthUpdate = obj->getWorldRenderData().renderDepthUpdated();
    bool texOrVertUpdate = obj->getWorldRenderData().textureOrVerticesUpdated();
    if (depthUpdate || texOrVertUpdate) {
        // add previous and current vertex coordinates to clipped update range
        obj->repaintRectangle(true);
    }
}

void Manager::registerOverheadMessage(boost::shared_ptr<OverheadMessage> msg) {
    overheadMessages_.push_back(msg);
}

void Manager::unregisterOverheadMessage(boost::shared_ptr<OverheadMessage> msg) {
    overheadMessages_.remove(msg);
}

void Manager::setAutoDeleteRange(unsigned int range) {
    autoDeleteRange_ = range + 2;
}

void Manager::addEffect(boost::shared_ptr<Effect> effect) {
    effects_.push_back(effect);
}

void Manager::systemMessage(const UnicodeString& msg, unsigned int hue, unsigned int font) {
    LOG_INFO << "SysMsg: " << msg << std::endl;

    sysLog_->add(msg, hue, font);

    // TODO: add to journal
}

std::list<boost::shared_ptr<world::Effect> >::iterator Manager::effectsBegin() {
    return effects_.begin();
}

std::list<boost::shared_ptr<world::Effect> >::iterator Manager::effectsEnd() {
    return effects_.end();
}

void Manager::clear() {
    player_.reset();
    mobiles_.clear();
    dynamicItems_.clear();
    smoothMovementManager_->clear();
    overheadMessages_.clear();
    effects_.clear();
    sectorManager_->clear();
}

void Manager::updateRoofHeight() {
    roofHeight_ = INT_MAX;
    unsigned int playerX = player_->getLocXGame();
    unsigned int playerY = player_->getLocYGame();
    int playerZ = player_->getLocZGame() + 15;

    // we need to check 4 map tiles (x/y, x+1/y, x/y+1, x+1/y+1) and the statics on (x/y and x+1/y+1)
    boost::shared_ptr<Sector> sectorXY = sectorManager_->getSectorForCoordinates(playerX, playerY);
    boost::shared_ptr<Sector> sectorX1Y = sectorManager_->getSectorForCoordinates(playerX + 1, playerY);
    boost::shared_ptr<Sector> sectorXY1 = sectorManager_->getSectorForCoordinates(playerX, playerY + 1);
    boost::shared_ptr<Sector> sectorX1Y1 = sectorManager_->getSectorForCoordinates(playerX + 1, playerY + 1);

    // map tile check
    if (sectorXY->getMapZAt(playerX, playerY) >= playerZ &&
            sectorX1Y->getMapZAt(playerX + 1, playerY) >= playerZ &&
            sectorXY1->getMapZAt(playerX, playerY + 1) >= playerZ &&
            sectorX1Y1->getMapZAt(playerX + 1, playerY + 1) >= playerZ) {
        // player is fully under map
        roofHeight_ = playerZ;
        return;
    }

    // map check is okay, now check all statics and dynamics
    std::list<world::IngameObject*> objList;
    sectorXY->getWalkObjectsOn(playerX, playerY, objList);
    std::list<world::IngameObject*>::const_iterator iter = objList.begin();
    std::list<world::IngameObject*>::const_iterator end = objList.end();

    for (; iter != end; ++iter) {
        int curZ = (*iter)->getLocZGame();
        if (curZ >= playerZ && curZ < roofHeight_) {
            if ((*iter)->isStaticItem()) {
                const data::StaticTileInfo* info = static_cast<world::StaticItem*>(*iter)->getTileDataInfo();
                if (!info->roof() && (info->surface() || info->impassable())) {
                    roofHeight_ = curZ;
                }
            } else if ((*iter)->isDynamicItem()) {
                const data::StaticTileInfo* info = static_cast<world::DynamicItem*>(*iter)->getTileDataInfo();
                if (!info->roof() && (info->surface() || info->impassable())) {
                    roofHeight_ = curZ;
                }
            }
        }
    }

    // now check x+1/y+1
    objList.clear();
    sectorX1Y1->getWalkObjectsOn(playerX + 1, playerY + 1, objList);
    iter = objList.begin();
    end = objList.end();

    for (; iter != end; ++iter) {
        int curZ = (*iter)->getLocZGame();
        if (curZ >= playerZ && curZ < roofHeight_) {
            if ((*iter)->isStaticItem()) {
                const data::StaticTileInfo* info = static_cast<world::StaticItem*>(*iter)->getTileDataInfo();
                if (info->roof()) {
                    roofHeight_ = playerZ;
                }
            } else if ((*iter)->isDynamicItem()) {
                const data::StaticTileInfo* info = static_cast<world::DynamicItem*>(*iter)->getTileDataInfo();
                if (info->roof()) {
                    roofHeight_ = playerZ;
                }
            }
        }
    }
}

int Manager::getRoofHeight() const {
    return roofHeight_;
}

void Manager::invalidateAllTextures() {
    sectorManager_->invalidateAllTextures();

    std::map<Serial, boost::shared_ptr<Mobile> >::iterator mobIter = mobiles_.begin();
    std::map<Serial, boost::shared_ptr<Mobile> >::iterator mobEnd = mobiles_.end();
    for (; mobIter != mobEnd; ++mobIter) {
        mobIter->second->invalidateTextureProvider();
    }

    std::map<Serial, boost::shared_ptr<DynamicItem> >::iterator itmIter = dynamicItems_.begin();
    std::map<Serial, boost::shared_ptr<DynamicItem> >::iterator itmEnd = dynamicItems_.end();
    for (; itmIter != itmEnd; ++itmIter) {
        itmIter->second->invalidateTextureProvider();
    }

    std::list<boost::shared_ptr<Effect> >::iterator effectIter = effects_.begin();
    std::list<boost::shared_ptr<Effect> >::iterator effectEnd = effects_.end();

    for (; effectIter != effectEnd; ++effectIter) {
        (*effectIter)->invalidateTextureProvider();
    }
}

void Manager::setWeather(unsigned int type, unsigned int intensity, unsigned int temperature) {
    if (weatherType_ != type) {
        // TODO: weather change, display message
        setCurrentWeatherEffect(type);
        setCurrentWeatherIntensity(intensity);
    } else if (weatherIntensity_ != intensity) {
        setCurrentWeatherIntensity(intensity);
    }
}

void Manager::setCurrentWeatherEffect(unsigned int type) {
    LOG_DEBUG << "Weather effect: " << type << std::endl;
    weatherType_ = type;
    if (weatherEffects_.first) {
        // store previous weather effect, and make it stop
        weatherEffects_.second = weatherEffects_.first;
        weatherEffects_.second->event("stop");
    }

    boost::shared_ptr<world::WeatherEffect> eff;
    switch (type) {
        case 0:
            // rain
            eff.reset(new world::WeatherEffect());
            if (ui::particles::XmlLoader::fromFile("rain", eff)) {
                weatherEffects_.first = eff;
            } else {
                weatherEffects_.first.reset();
            }
            break;
        case 1:
            // storm approaches, no effect
            weatherEffects_.first.reset();
            break;
        case 2:
            // snow
            eff.reset(new world::WeatherEffect());
            if (ui::particles::XmlLoader::fromFile("snow", eff)) {
                weatherEffects_.first = eff;
            } else {
                weatherEffects_.first.reset();
            }
            break;
        case 3:
            // storm brewing, no effect
            weatherEffects_.first.reset();
            break;
        default:
            // reset weather, no effect
            weatherEffects_.first.reset();
            break;
    }
}

void Manager::setCurrentWeatherIntensity(unsigned int intensity) {
    if (intensity > 70) {
        intensity = 70;
    }

    LOG_DEBUG << "Weather intensity: " << intensity << std::endl;
    weatherIntensity_ = intensity;
    if (weatherEffects_.first) {
        unsigned int intensityRnd = (intensity / 10) * 10;
        UnicodeString intensityStr("intensity");
        intensityStr += StringConverter::fromNumber(intensityRnd);
        weatherEffects_.first->event(intensityStr);
    }
}

std::pair<boost::shared_ptr<world::WeatherEffect>, boost::shared_ptr<world::WeatherEffect> > Manager::getWeatherEffects() {
    return weatherEffects_;
}

}
}
