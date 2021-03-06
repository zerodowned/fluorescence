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


#ifndef FLUO_DATA_WEAKPTRCACHE_HPP
#define FLUO_DATA_WEAKPTRCACHE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <map>
#include <list>

#include <misc/log.hpp>

namespace fluo {
namespace data {

/**
 * \brief This class provides caching based on weak pointers. Additionally, it provides support for fixing items in the memory
 */
template <
typename KeyType,
typename ValueType,
template <typename, typename> class FileLoader
>
class WeakPtrCache {
private:
    typedef std::map<KeyType, boost::weak_ptr<ValueType> > MapType;
    typedef std::list<boost::shared_ptr<ValueType> > FixedListType;

public:
    WeakPtrCache() {
    }

    void init(boost::shared_ptr<FileLoader<KeyType, ValueType> > loader) {
        loader_ = loader;
    }

    /// Fixes an item in the memory
    void fixItem(const KeyType& id) {
        boost::shared_ptr<ValueType> toFix = get(id);
        fixedList_.push_back(toFix);
    }

    /// Fixes an item in the memory
    void fixItem(boost::shared_ptr<ValueType>& item) {
        fixedList_.push_back(item);
    }

    /// Removes the memory fixation for a given id
    void removeFixed(const KeyType& id) {
        boost::shared_ptr<ValueType> toRemove = get(id);
        fixedList_.remove(toRemove);
    }

    /// Removes the memory fixation for a given item
    void removeFixed(boost::shared_ptr<ValueType>& item) {
        fixedList_.remove(item);
    }

    boost::shared_ptr<ValueType> getNoCreate(const KeyType& id) {
        typename MapType::iterator iter = this->cache_.find(id);
        if (iter != this->cache_.end()) {
            // was loaded at some point
            return iter->second.lock();
        } else {
            return boost::shared_ptr<ValueType>();
        }
    }


    boost::shared_ptr<ValueType> get(const KeyType& id, unsigned int userData = 0) {
        typename MapType::iterator iter = this->cache_.find(id);
        if (iter != this->cache_.end()) {
            // was loaded at some point - is it still valid?
            boost::shared_ptr<ValueType> smPtr = iter->second.lock();
            if (smPtr.get() != NULL) {
                return smPtr;
            } else {
                // we need to reload it
                return this->load(id, userData);
            }
        } else {
            // we need to load it for the first time
            return this->load(id, userData);
        }
    }

    bool hasId(const KeyType& id) {
        return cache_.find(id) != cache_.end();
    }

    void printStats() {
        unsigned int weakCount = cache_.size();
        unsigned int realCount = 0;
        unsigned int avgWidth = 0;
        unsigned int avgHeight = 0;
        unsigned int maxWidth = 0;
        unsigned int maxHeight = 0;

        typename MapType::iterator iter = cache_.begin();
        typename MapType::iterator end = cache_.end();

        for (; iter != end; ++iter) {
            // was loaded at some point - is it still valid?
            boost::shared_ptr<ValueType> smPtr = iter->second.lock();
            if (smPtr) {
                realCount += 1;
                unsigned int width = smPtr->getWidth();
                unsigned int height = smPtr->getHeight();

                avgWidth += width;
                avgHeight += height;

                maxWidth = (std::max)(maxWidth, width);
                maxHeight = (std::max)(maxHeight, height);
            }
        }

        LOG_DEBUG << "WeakPtrCache stats: weak=" << weakCount << " real=" << realCount << std::endl;
        LOG_DEBUG << "Sizes avg=" << (avgWidth / realCount) << "/" << (avgHeight / realCount) << "  max=" << maxWidth << "/" << maxHeight << std::endl;
    }

    void clear() {
        fixedList_.clear();
        cache_.clear();
    }

private:
    MapType cache_;
    FixedListType fixedList_; ///< Keeps a shared_ptr to fixed items. May contain duplicates

    boost::shared_ptr<FileLoader<KeyType, ValueType> > loader_;

    boost::shared_ptr<ValueType> load(const KeyType& id, unsigned int userData) {
        boost::shared_ptr<ValueType> smPtr = loader_->get(id, userData);
        cache_[id] = boost::weak_ptr<ValueType>(smPtr);
        return smPtr;
    }
};

}
}


#endif
