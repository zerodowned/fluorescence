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

#include "pyclient.hpp"

#include <client.hpp>
#include <misc/log.hpp>

#include <net/manager.hpp>
#include <net/packets/00_createcharacter.hpp>
#include <net/packets/12_useskill.hpp>
#include <net/packets/9b_helprequest.hpp>
#include <net/packets/7d_objectpickerresponse.hpp>
#include <net/packets/83_characterdelete.hpp>
#include <net/packets/bf.hpp>
#include <net/packets/bf/15_contextmenureply.hpp>
#include <net/packets/bf/1c_castspell.hpp>

#include <ui/manager.hpp>
#include <ui/commandmanager.hpp>
#include <ui/gumpmenus.hpp>

namespace fluo {
namespace ui {
namespace python {

void PyClient::setShard(const UnicodeString& name) {
    Client::getSingleton()->selectShard(name);
}

bool PyClient::createShard(const UnicodeString& name, const UnicodeString& path, bool highSeas) {
    return Client::getSingleton()->createShard(name, path, highSeas);
}

bool PyClient::connect(const UnicodeString& host, unsigned int port, const UnicodeString& account, const UnicodeString& password) {
    return Client::getSingleton()->connect(host, port, account, password);
}

UnicodeString PyClient::getConfig(const UnicodeString& key) {
    if (Client::getSingleton()->getConfig().exists(key)) {
        return Client::getSingleton()->getConfig()[key].asString();
    } else {
        LOG_WARN << "Requested unknown config value: " << key << std::endl;
        return "";
    }
}

void PyClient::shutdown() {
    Client::getSingleton()->shutdown();
}

void PyClient::messagebox(const UnicodeString& msg) {
    ui::GumpMenus::openMessageBox(msg);
}

void PyClient::openGump(const UnicodeString& name) {
    ui::Manager::getSingleton()->openPythonGump(name);
}

void PyClient::openGumpArgs(const UnicodeString& name, boost::python::dict args) {
    ui::Manager::getSingleton()->openPythonGump(name, args);
}

void PyClient::disconnect() {
    Client::getSingleton()->disconnect();
}

void PyClient::selectServer(unsigned int index) {
    net::Manager::getSingleton()->selectServer(index);
}

void PyClient::selectCharacter(unsigned int index, const UnicodeString& name, const UnicodeString& password) {
    net::Manager::getSingleton()->selectCharacter(index, name, password);
}

void PyClient::deleteCharacter(unsigned int index, const UnicodeString& password) {
    net::packets::CharacterDelete reply(password, index, net::Manager::getSingleton()->getSeed());
    net::Manager::getSingleton()->send(reply);
}

void PyClient::createDummyCharacter() {
    net::packets::CreateCharacter pkt;

    pkt.name_ = "Dummy";
    pkt.slot_ = 0;
    pkt.startCity_ = 1;

    net::Manager::getSingleton()->send(pkt);
}

void PyClient::handleTextInput(const UnicodeString& text) {
    ui::Manager::getSingleton()->handleTextInput(text);
}

void PyClient::sendHelpRequest() {
    net::packets::HelpRequest pkt;
    net::Manager::getSingleton()->send(pkt);
}

void PyClient::useSkill(unsigned int id) {
    net::packets::UseSkill pkt(id);
    net::Manager::getSingleton()->send(pkt);
}

void PyClient::castSpell(unsigned int spellId) {
    boost::shared_ptr<net::Packet> subPacket(new net::packets::bf::CastSpell(spellId));
    net::packets::BF pkt(subPacket);
    net::Manager::getSingleton()->send(pkt);
}

void PyClient::objectPickerResponse(unsigned int serial, unsigned int menuId, unsigned int answerId, unsigned int artId, unsigned int hue) {
    net::packets::ObjectPickerResponse pkt(serial, menuId, answerId, artId, hue);
    net::Manager::getSingleton()->send(pkt);
}

void PyClient::contextMenuReply(unsigned int serial, unsigned int replyId) {
    boost::shared_ptr<net::Packet> subPacket(new net::packets::bf::ContextMenuReply(serial, replyId));
    net::packets::BF pkt(subPacket);
    net::Manager::getSingleton()->send(pkt);
}

}
}
}

