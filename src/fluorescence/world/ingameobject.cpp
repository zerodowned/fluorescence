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



#include "ingameobject.hpp"

#include <stdio.h>
#include <iomanip>

#include <misc/log.hpp>

#include <ui/texture.hpp>
#include <ui/manager.hpp>
#include <ui/render/renderqueue.hpp>
#include <ui/render/material.hpp>
#include <ui/cliprectmanager.hpp>

#include "manager.hpp"
#include "overheadmessage.hpp"
#include "sectormanager.hpp"
#include "sector.hpp"

namespace fluo {
namespace world {

IngameObject::IngameObject(unsigned int objectType) :
    draggable_(false),
    materialInfo_(ui::render::MaterialInfo::get(Material::DEFAULT)),
    objectType_(objectType), visible_(true), ignored_(false), mouseOver_(false) {

}

IngameObject::~IngameObject() {
}

bool IngameObject::isVisible() const {
    return visible_ && !ignored_;
}

void IngameObject::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        forceRepaint();
    }
}

void IngameObject::setIgnored(bool ignored) {
    ignored_ = ignored;
}

void IngameObject::setLocation(float locX, float locY, float locZ) {
    setLocation(CL_Vec3f(locX, locY, locZ));
}

void IngameObject::setLocation(CL_Vec3f loc) {
    CL_Vec3f oldLocation = location_;

    location_ = loc;

    //LOGARG_DEBUG(LOGTYPE_WORLD, "Object location: %f %f %f", locX, locY, locZ);

    if (ceilf(oldLocation[0u]) != ceilf(location_[0u]) ||
            ceilf(oldLocation[1u]) != ceilf(location_[1u]) ||
            ceilf(oldLocation[2u]) != ceilf(location_[2u])) {

        invalidateRenderDepth();

        onLocationChanged(oldLocation);
    }

    invalidateVertexCoordinates();
}

float IngameObject::getLocXDraw() const {
    return location_[0u];
}

float IngameObject::getLocYDraw() const {
    return location_[1u];
}

float IngameObject::getLocZDraw() const {
    return location_[2u];
}

unsigned int IngameObject::getLocXGame() const {
    return ceilf(location_[0u]);
}

unsigned int IngameObject::getLocYGame() const {
    return ceilf(location_[1u]);
}

int IngameObject::getLocZGame() const {
    return ceilf(location_[2u]);
}

CL_Vec3f IngameObject::getLocation() const {
    return location_;
}

void IngameObject::invalidateTextureProvider() {
    worldRenderData_.invalidateTextureProvider();
    worldRenderData_.invalidateVertexCoordinates();
}

void IngameObject::invalidateVertexCoordinates() {
    worldRenderData_.invalidateVertexCoordinates();

    if (!childObjects_.empty()) {
        std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
        std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

        for (; iter != end; ++iter) {
            (*iter)->invalidateVertexCoordinates();
        }
    }
}

void IngameObject::invalidateRenderDepth() {
    worldRenderData_.invalidateRenderDepth();

    if (!childObjects_.empty()) {
        std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
        std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

        for (; iter != end; ++iter) {
            (*iter)->invalidateRenderDepth();
        }
    }
}

const CL_Vec3f* IngameObject::getVertexCoordinates() const {
    return worldRenderData_.getVertexCoordinates();
}

const CL_Vec3f& IngameObject::getHueInfo(bool ignoreMouseOver) const {
    if (mouseOver_ && !ignoreMouseOver && isDynamicItem()) {
        static CL_Vec3f mouseOverHueVec = CL_Vec3f(0, 0x9A, 1);
        return mouseOverHueVec;
    }

    return worldRenderData_.hueInfo_;
}

void IngameObject::updateRenderData(unsigned int elapsedMillis) {
    worldRenderData_.resetPreUpdate();

    if (worldRenderData_.renderDataValid()) {
        bool frameChanged = updateAnimation(elapsedMillis);

        if (frameChanged) {
            updateVertexCoordinates();
            worldRenderData_.onVertexCoordinatesUpdate();
            notifyRenderQueuesWorldTexture();
        }
    } else {
        if (worldRenderData_.textureProviderUpdateRequired()) {
            updateTextureProvider();
            worldRenderData_.onTextureProviderUpdate();
            notifyRenderQueuesWorldTexture();
        }

        boost::shared_ptr<ui::Texture> tex = getIngameTexture();
        if (!tex || !tex->isReadComplete()) {
            return;
        }

        bool frameChanged = updateAnimation(elapsedMillis);
        if (frameChanged || worldRenderData_.vertexCoordinatesUpdateRequired()) {
            updateVertexCoordinates();
            worldRenderData_.onVertexCoordinatesUpdate();

            if (frameChanged) {
                notifyRenderQueuesWorldTexture();
            } else {
                notifyRenderQueuesWorldCoordinates();
            }
        }

        if (worldRenderData_.renderDepthUpdateRequired()) {
            updateRenderDepth();
            worldRenderData_.onRenderDepthUpdate();
            notifyRenderQueuesWorldDepth();

            if (sector_) {
                sector_->requestSort();
            }
        }
    }
}

bool IngameObject::overlaps(const CL_Rectf& rect) const {
    //LOGARG_DEBUG(LOGTYPE_WORLD, "isInDrawArea (%u %u %u %u) => x=%u y=%u\n", leftPixelCoord, rightPixelCoord, topPixelCoord, bottomPixelCoord, vertexCoordinates_[0u].x, vertexCoordinates_[0u].y);

    // almost every texture is a rectangle, with vertexCoordinates_[0] being top left and vertexCoordinates_[5] bottom right
    // all non-rectangular cases are maptiles
    return (worldRenderData_.vertexCoordinates_[0].x <= rect.right) &&
            (worldRenderData_.vertexCoordinates_[0].y <= rect.bottom) &&
            (worldRenderData_.vertexCoordinates_[5].x >= rect.left) &&
            (worldRenderData_.vertexCoordinates_[5].y >= rect.top);
}

bool IngameObject::hasWorldPixel(int pixelX, int pixelY) const {
    // almost every texture is a rectangle, with vertexCoordinates_[0] being top left and vertexCoordinates_[5] bottom right
    // all non-rectangular cases are maptiles
    bool coordinateCheck = worldRenderData_.vertexCoordinates_[0].x <= pixelX &&
            worldRenderData_.vertexCoordinates_[0].y <= pixelY &&
            worldRenderData_.vertexCoordinates_[5].x >= pixelX &&
            worldRenderData_.vertexCoordinates_[5].y >= pixelY;

    boost::shared_ptr<ui::Texture> tex = getIngameTexture();
    if (coordinateCheck && tex && tex->isReadComplete()) {
        unsigned int texPixelX = pixelX - worldRenderData_.vertexCoordinates_[0].x;
        unsigned int texPixelY = pixelY - worldRenderData_.vertexCoordinates_[0].y;

        return tex->hasPixel(texPixelX, texPixelY);
    } else {
        return false;
    }
}

bool IngameObject::hasContainerPixel(int pixelX, int pixelY) const {
    int gameX = getLocXGame();
    int gameY = getLocYGame();
    if (pixelX < gameX || pixelY < gameY) {
        return false;
    }

    boost::shared_ptr<ui::Texture> tex = getIngameTexture();
    if (tex && tex->isReadComplete()) {
        pixelX -= gameX;
        pixelY -= gameY;

        return pixelX < tex->getWidth() && pixelY < tex->getHeight() && tex->hasPixel(pixelX, pixelY);
    }

    return false;
}

bool IngameObject::hasGumpPixel(int pixelX, int pixelY) const {
    boost::shared_ptr<ui::Texture> tex = getGumpTexture();
    if (tex && tex->isReadComplete()) {
        return tex->hasPixel(pixelX, pixelY);
    } else {
        return false;
    }
}

const CL_Vec3f* IngameObject::getVertexNormals() const {
    return worldRenderData_.vertexNormals_;
}

void IngameObject::onClick() {
}

void IngameObject::onDoubleClick() {
}

bool IngameObject::isMirrored() const {
    return false;
}

bool IngameObject::isDraggable() const {
    return draggable_;
}

void IngameObject::onDraggedOnto(boost::shared_ptr<IngameObject> obj, int locX, int locY) {
    LOG_ERROR << "Undraggable object dragged on other object" << std::endl;
}

void IngameObject::onDraggedToVoid() {
    LOG_ERROR << "Undraggable object dragged on void" << std::endl;
}

void IngameObject::onStartDrag(const CL_Point& mousePos) {
    LOG_ERROR << "Starting to drag object other than DynamicItem or Mobile" << std::endl;
}

boost::shared_ptr<IngameObject> IngameObject::getTopParent() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();
    if (parent) {
        return parent->getTopParent();
    } else {
        return shared_from_this();
    }
}

void IngameObject::printRenderDepth() const {
    LOG_DEBUG << "Render depth: " << std::hex << worldRenderData_.getRenderDepth().value_ << std::dec << std::endl;
}

void IngameObject::setOverheadMessageOffsets() {
    std::list<boost::shared_ptr<IngameObject> >::reverse_iterator iter = childObjects_.rbegin();
    std::list<boost::shared_ptr<IngameObject> >::reverse_iterator end = childObjects_.rend();

    int offset = -5;

    for (; iter != end; ++iter) {
        if ((*iter)->isSpeech()) {
            boost::shared_ptr<OverheadMessage> msgObj = boost::static_pointer_cast<OverheadMessage>(*iter);
            int curHeight = (*iter)->getIngameTexture()->getHeight();
            int curOffset = offset - curHeight;
            msgObj->setParentPixelOffset(curOffset);
            offset = curOffset - 2;
        }
    }
}

bool IngameObject::isInRenderQueue(const boost::shared_ptr<ui::RenderQueue>& rq) {
    return std::find(renderQueues_.begin(), renderQueues_.end(), rq) != renderQueues_.end();
}

void IngameObject::addToRenderQueue(const boost::shared_ptr<ui::RenderQueue>& rq) {
    if (!isInRenderQueue(rq)) {
        renderQueues_.push_back(rq);
        rq->add(shared_from_this());

        if (!childObjects_.empty()) {
            std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
            std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

            for (; iter != end; ++iter) {
                if ((*iter)->isSpeech() || isMobile()) {
                    (*iter)->addToRenderQueue(rq);
                }
            }
        }
    }
}

void IngameObject::removeFromRenderQueue(const boost::shared_ptr<ui::RenderQueue>& rq) {
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator iter = std::find(renderQueues_.begin(), renderQueues_.end(), rq);

    if (iter != renderQueues_.end()) {
        renderQueues_.erase(iter);
        rq->remove(shared_from_this());

        if (!childObjects_.empty()) {
            std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
            std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

            for (; iter != end; ++iter) {
                if ((*iter)->isSpeech() || isMobile()) {
                    (*iter)->removeFromRenderQueue(rq);
                }
            }
        }
    }
}

void IngameObject::removeFromAllRenderQueues() {
    std::list<boost::shared_ptr<ui::RenderQueue> > rqCopy(renderQueues_.begin(), renderQueues_.end());
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator iter = rqCopy.begin();
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator end = rqCopy.end();

    boost::shared_ptr<IngameObject> sharedThis = shared_from_this();
    for (; iter != end; ++iter) {
        removeFromRenderQueue(*iter);
    }
}

void IngameObject::addChildObject(boost::shared_ptr<IngameObject> obj) {
    std::list<boost::shared_ptr<IngameObject> >::iterator iter = std::find(childObjects_.begin(), childObjects_.end(), obj);

    if (iter == childObjects_.end()) {
        childObjects_.push_back(obj);
        obj->setParentObject(shared_from_this());
        onChildObjectAdded(obj);

        if (obj->isSpeech()) {
            setOverheadMessageOffsets();
        }
    }
}

void IngameObject::removeChildObject(boost::shared_ptr<IngameObject> obj) {
    std::list<boost::shared_ptr<IngameObject> >::iterator iter = std::find(childObjects_.begin(), childObjects_.end(), obj);

    if (iter != childObjects_.end()) {
        onBeforeChildObjectRemoved(*iter);
        obj->setParentObject();
        childObjects_.erase(iter);
        onAfterChildObjectRemoved();
    }
}

void IngameObject::onAddedToParent() {
}

void IngameObject::onRemovedFromParent() {
}

void IngameObject::onChildObjectAdded(const boost::shared_ptr<IngameObject>& obj) {
}

void IngameObject::onAfterChildObjectRemoved() {
}

void IngameObject::onBeforeChildObjectRemoved(const boost::shared_ptr<IngameObject>& obj) {
}

void IngameObject::onAddedToSector(world::Sector* sector) {
}

void IngameObject::onRemovedFromSector(world::Sector* sector) {
}

void IngameObject::setSector(boost::shared_ptr<world::Sector> sector) {
    sector_ = sector;
    sector_->addDynamicObject(this);
}

void IngameObject::onLocationChanged(const CL_Vec3f& oldLocation) {
}

void IngameObject::setParentObject() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();

    if (parent) {
        onRemovedFromParent();
        parentObject_.reset();
    }
}

void IngameObject::setParentObject(boost::shared_ptr<IngameObject> parent) {
    boost::shared_ptr<IngameObject> curParent = parentObject_.lock();
    if (curParent) {
        setParentObject();
    }

    parentObject_ = parent;
    onAddedToParent();
}

bool IngameObject::isMap() const {
    return objectType_ == TYPE_MAP;
}

bool IngameObject::isStaticItem() const {
    return objectType_ == TYPE_STATIC_ITEM;
}

bool IngameObject::isDynamicItem() const {
    return objectType_ == TYPE_DYNAMIC_ITEM;
}

bool IngameObject::isMobile() const {
    return objectType_ == TYPE_MOBILE;
}

bool IngameObject::isSpeech() const {
    return objectType_ == TYPE_SPEECH;
}

bool IngameObject::isParticleEffect() const {
    return objectType_ == TYPE_PARTICLE_EFFECT;
}

bool IngameObject::isOsiEffect() const {
    return objectType_ == TYPE_OSI_EFFECT;
}

void IngameObject::onDelete() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();
    if (parent) {
        parent->removeChildObject(shared_from_this());
    }

    std::list<boost::shared_ptr<ui::RenderQueue> > rqsToRemove(renderQueues_.begin(), renderQueues_.end());
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = rqsToRemove.begin();
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = rqsToRemove.end();
    for (; rqIter != rqEnd; ++rqIter) {
        removeFromRenderQueue(*rqIter);
    }
}

boost::shared_ptr<ui::Texture> IngameObject::getGumpTexture() const {
    LOG_ERROR << "getGumpTexture called on IngameObject" << std::endl;
    return getIngameTexture();
}


void IngameObject::notifyRenderQueuesWorldTexture() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldTextureChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldTextureChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesWorldCoordinates() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldCoordinatesChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldCoordinatesChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesWorldDepth() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldDepthChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldDepthChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesGump() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onGumpChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onGumpChanged();
        }
    }
}

void IngameObject::forceRepaint() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->forceRepaint();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->forceRepaint();
        }
    }

    repaintRectangle();
}

std::list<boost::shared_ptr<ui::RenderQueue> >::iterator IngameObject::rqBegin() {
    return renderQueues_.begin();
}

std::list<boost::shared_ptr<ui::RenderQueue> >::iterator IngameObject::rqEnd() {
    return renderQueues_.end();
}

const ui::WorldRenderData& IngameObject::getWorldRenderData() const {
    return worldRenderData_;
}

ui::WorldRenderData& IngameObject::getWorldRenderData() {
    return worldRenderData_;
}

void IngameObject::updateGumpTextureProvider() {
}

const RenderDepth& IngameObject::getRenderDepth() const {
    return worldRenderData_.getRenderDepth();
}

bool IngameObject::renderDepthChanged() const {
    return worldRenderData_.renderDepthUpdated();
}

bool IngameObject::textureOrVerticesChanged() const {
    return worldRenderData_.textureOrVerticesUpdated();
}

void IngameObject::repaintRectangle(bool repaintPreviousCoordinates) const {
    if (repaintPreviousCoordinates) {
        ui::Manager::getClipRectManager()->add(worldRenderData_.previousVertexRect_);
    }
    ui::Manager::getClipRectManager()->add(worldRenderData_.getCurrentVertexRect());
}

bool IngameObject::hasParent() const {
    return !parentObject_.expired();
}

void IngameObject::setMaterial(unsigned int material) {
    materialInfo_ = ui::render::MaterialInfo::get(material);
}

const ui::render::MaterialInfo* IngameObject::getMaterial() const {
    return materialInfo_;
}

void IngameObject::setMouseOver(bool mo) {
    mouseOver_ = mo;
    forceRepaint();
}

void IngameObject::openPropertyListGump(const CL_Point& mousePos) {
}

}
}
