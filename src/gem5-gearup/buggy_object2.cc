#include "gem5-gearup/buggy_object2.hh"

namespace gem5
{

BuggyObject2::BuggyObject2(const BuggyObject2Params &params) :
    SimObject(params),
    icache_port(params.name + ".icache_port", this),
    dcache_port(params.name + ".dcache_port", this),
    mem_port(params.name + ".mem_port", this)
{
}

Port &
BuggyObject2::getPort(const std::string &if_name, PortID idx)
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
BuggyObject2::handleRequest(PacketPtr pkt, bool timing)
{
    if (timing && !mem_port.sendTimingReq(pkt)) {
        mem_port.blockedPackets.push_back(pkt);
    } else if (!timing) {
        mem_port.sendFunctional(pkt);
    }

    return true;
}

bool
BuggyObject2::handleResponse(PacketPtr pkt)
{
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
