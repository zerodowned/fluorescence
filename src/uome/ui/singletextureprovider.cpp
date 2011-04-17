
#include "singletextureprovider.hpp"

#include <data/manager.hpp>
#include <data/artloader.hpp>

namespace uome {
namespace ui {

SingleTextureProvider::SingleTextureProvider(unsigned int artId_) {
    texture_ = data::Manager::getArtLoader()->getItemTexture(artId_);
}

boost::shared_ptr<ui::Texture> SingleTextureProvider::getTexture() {
    return texture_;
}

void SingleTextureProvider::update(unsigned int elapsedMillis) {
    // do nothing
}

}
}