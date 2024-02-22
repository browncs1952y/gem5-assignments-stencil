#include "gem5-gearup/encryption_object.hh"

namespace gem5
{

EncryptionObject::EncryptionObject(const EncryptionObjectParams &params) :
    SimObject(params),
    icache_port(params.name + ".icache_port", this),
    dcache_port(params.name + ".dcache_port", this),
    mem_port(params.name + ".mem_port", this)
{
    encryption_seed = 0xff & rand();
}

Port &
EncryptionObject::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "mem_port") {
        return mem_port;
    } else if (if_name == "icache_port") {
        return icache_port;
    } else if (if_name == "dcache_port") {
        return dcache_port;
    }

    return SimObject::getPort(if_name, idx);
}

bool
EncryptionObject::handleRequest(PacketPtr pkt, bool timing)
{
    if (pkt->isWrite()) {
        uint8_t *ciphertext = (uint8_t *) malloc(sizeof(uint8_t) * pkt->getSize());
        pkt->writeData(ciphertext);

        for (int i = 0; i < pkt->getSize(); i++) {
            ciphertext[i] ^= encryption_seed;
        }

        pkt->deleteData();
        pkt->dataStatic(ciphertext); 

        encrypted_addrs.push_back(pkt->getAddr());
    }

    if (timing && !mem_port.sendTimingReq(pkt)) {
        mem_port.blockedPackets.push_back(pkt);
    } else if (!timing) {
        mem_port.sendFunctional(pkt);
    }

    return true;
}

bool
EncryptionObject::handleResponse(PacketPtr pkt)
{
    if (pkt->isRead()) {
        uint8_t *ciphertext = (uint8_t *) malloc(sizeof(uint8_t) * pkt->getSize());
        pkt->writeData(ciphertext);

        for (int i = 0; i < pkt->getSize(); i++) {
            ciphertext[i] ^= encryption_seed;
        }

        pkt->deleteData();
        pkt->dataStatic(ciphertext);
    }

    if (pkt->req->isInstFetch()) {
        if (!icache_port.sendTimingResp(pkt)) {
            icache_port.blockedPackets.push_back(pkt);
        }
    } else {
        if (!dcache_port.sendTimingResp(pkt)) {
            dcache_port.blockedPackets.push_back(pkt);
        }
    }

    return true;
}

};
