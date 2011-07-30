#ifndef UOME_NET_PACKETS_DELETEOBJECT_HPP
#define UOME_NET_PACKETS_DELETEOBJECT_HPP

#include <net/packet.hpp>

#include <typedefs.hpp>

namespace uome {
namespace net {

namespace packets {

class DeleteObject : public Packet {
public:
    DeleteObject();

    virtual bool read(const int8_t* buf, unsigned int len, unsigned int& index);

    virtual void onReceive();

    Serial serial_;
};

}
}
}

#endif
