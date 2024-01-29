/*
 * Author: YOUR NAME HERE!
 * Copyright (c) 2023 Brown University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/micro-cache.hh"
#include "params/MicroCache.hh"

namespace gem5 {

MicroCache::MicroCache(const MicroCacheParams *p) :
    SimObject(*p),
    cpu_side_port(p->name + ".cpu_side_port", this),
    mem_side_port(p->name + ".mem_side_port", this),
    memSendEvent([this] { sendToMem(); }, name()),
    cpuSendEvent([this] { sendToCpu(); }, name()),
    writebackEvent([this] { writebackToMem(); }, name()),
    // TODO: YOUR ADDITIONAL FIELDS HERE!
    blocked(false),
    to_mem(nullptr),
    to_cpu(nullptr),
    to_writeback(nullptr),
    latency(p->latency * 1000)
{
};


Port&
MicroCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "cpu_side") {
        return cpu_side_port;
    } else if (if_name == "mem_side") {
        return mem_side_port;
    }

    return SimObject::getPort(if_name, idx);
}


bool
MicroCache::handleRequest(PacketPtr pkt)
{
    if (blocked) {
        return false;
    }

    blocked = true;

    // TODO: Your implementation here!
    if (false) {
        // hit case!

        // TODO!


        if (pkt->needsResponse()) {
            pkt->makeTimingResponse();
            to_cpu = pkt;

            schedule(cpuSendEvent, curTick() + latency);
        }
    } else {
        // miss case!
        pending = pkt;

        // TODO!
    }

    return true;
}


void
MicroCache::handleResponse(PacketPtr pkt)
{
    assert(blocked);

    if (false /* some check if we need to writeback data */)
        writebackData(false /* dirty? */, 0 /* tags info */, nullptr /* data */);

    // this packet was dynamically created by us
    delete pkt;

    // TODO!

    if (pending->needsResponse()) {
        pending->makeTimingResponse();
        to_cpu = pending;

        schedule(cpuSendEvent, curTick() + latency);
    }

    pending = nullptr;
}

gem5::MicroCache*
MicroCacheParams::create() const
{
    return new gem5::MicroCache(this);
}

}
