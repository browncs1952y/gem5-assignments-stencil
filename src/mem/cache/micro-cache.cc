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
     unblockEvent([this] { unblock(); }, name()),
     toMem(nullptr),
     toCPU(nullptr),
     toWriteback(nullptr),
     latency(p->latency * 1000),
     assoc(p->assoc),
     blocked(false),
     writingback(false),
     pending(nullptr),
     stats(this)
     // TODO: YOUR ADDITIONAL FIELDS HERE!
 {
     assert(p->size >= 64);

     // allocate one block;
     blks = (Block *) malloc(sizeof(Block));
     blks[0].valid = false;
 
 };
 
 /* Method required for Cache object to communicate */
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
 
 /* Request from CPU side */
 bool
 MicroCache::handleRequest(PacketPtr pkt)
 { 
     if (blocked) {
         return false;
     }
     blocked = true;
 
 
     if (false) { // TODO: check if address is in cache
         // hit case!
         stats.hits++; // scaff

         // todo: check information about pkt
 
         if (pkt->needsResponse()) {
            // todo: populate pkt with appropriate data
             pkt->makeTimingResponse();
             toCPU = pkt;
 
             // schedule response event to be sent
             schedule(cpuSendEvent, curTick() + latency);
         } else {
             schedule(unblockEvent, curTick() + latency);
         }
     } else if (false) { // TODO: check if address is not in cache
         assert(pkt->isRead() || pkt->isWrite());
         // miss case!
         stats.misses++;
 
         // TODO: send a request to memory using requestFromMem
 
         pending = pkt; // make sure we track the request we sent
     } else {
         // Ignore non-read, non-write packets
         blocked = false;
     }
 
     return true;
 }
 
 
 void
 MicroCache::handleResponse(PacketPtr pkt)
 {
    assert(pending != nullptr);
 
    Addr pendingAddr = pending->getAddr();
 
     if (writingback) {
        // this is an acknowledgement that writeback is complete!
         writingback = false;
         assert(pkt->isWrite() && pkt->isResponse());
     } else {
        // TODO: where does the data go?

         if (false) { // TODO: do we need to evict a block?
            // TODO: call writebackData

             writingback = true;
         }
 
         // TODO: fill block using data in plt

     }
 
     // respond to CPU if necessary and unblock
     if (!writingback) {
         assert(blocked);
         if (pending->needsResponse()) {
            // make sure pending data is set here or above!
             pending->makeTimingResponse();
             toCPU = pending;
             pending = nullptr;
 
             // schedule response event to be sent
             schedule(cpuSendEvent, curTick() + latency);
         } else {
             pending = nullptr;
             schedule(unblockEvent, curTick() + latency);
         }
     }
 
     // this packet was dynamically created by us
     delete pkt; // scaff
 }
 
 MicroCache::MicroCacheStats::MicroCacheStats(statistics::Group *parent)
   : statistics::Group(parent),
     ADD_STAT(hits, statistics::units::Count::get(), "Number of hits"),
     ADD_STAT(misses, statistics::units::Count::get(), "Number of misses"),
     ADD_STAT(hitRate, statistics::units::Ratio::get(), "Number of hits/ (hits + misses)", hits / (hits + misses))
 
 {
 }
 
 gem5::MicroCache*
 MicroCacheParams::create() const
 {
     return new gem5::MicroCache(this);
 }
 
 }
 