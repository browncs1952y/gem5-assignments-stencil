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

/**
 * @file
 * TODO: FILE DESCRIPTION HERE!
 */

 #ifndef __MICRO_CACHE_HH__
 #define __MICRO_CACHE_HH__
 
 #include "base/statistics.hh"
 #include "mem/port.hh"
 #include "sim/sim_object.hh"
 
 #include "debug/MicroCache.hh"
 
 namespace gem5 {
 
 struct MicroCacheParams;
 
 class MicroCache : public SimObject
 {
   private:
     class CpuSidePort : public ResponsePort
     {
       private:
         MicroCache *owner;
 
       public:
         bool needRetry;
 
         CpuSidePort(const std::string &name, MicroCache *owner) :
             ResponsePort(name), owner(owner), needRetry(false)
           {  };
 
       protected:
         bool recvTimingReq(PacketPtr pkt) override {
             bool ret = owner->handleRequest(pkt);
             if (!ret) needRetry = true;
             if (needRetry) DPRINTF(MicroCache, "Retry needed\n");
             return ret;
         };
 
         // un-used for assignment, do not modify!
         Tick recvAtomic(PacketPtr pkt) override {
             return owner->mem_side_port.sendAtomic(pkt);
         }
 
         void recvRespRetry() override {
             if (needRetry) {
                 needRetry = false;
                 sendRetryReq();
             }
          };
 
         void recvFunctional(PacketPtr pkt) override {
             owner->mem_side_port.sendFunctional(pkt);
         };
 
         AddrRangeList getAddrRanges() const override {
             return owner->mem_side_port.getAddrRanges();
         };
 
     };
 
     class MemSidePort : public RequestPort
     {
       private:
         MicroCache *owner;
 
       public:
         PacketPtr blocked_pkt;
 
         MemSidePort(const std::string &name, MicroCache *owner) :
                     RequestPort(name), owner(owner), blocked_pkt(nullptr)
         {  };
 
         void sendPacket(PacketPtr pkt) {
             assert(blocked_pkt == nullptr);
 
             if (!sendTimingReq(pkt)) {
                 blocked_pkt = pkt;
             }
         }
 
       protected:
         bool recvTimingResp(PacketPtr pkt) override {
             owner->handleResponse(pkt);
             return true;
         };
 
         void recvReqRetry() override {
             assert(blocked_pkt != nullptr);
             PacketPtr pkt = blocked_pkt;
             blocked_pkt = nullptr;
             sendTimingReq(pkt);
         };
 
         void recvRangeChange() override {
             owner->cpu_side_port.sendRangeChange();
         }
 
         void recvTimingSnoopReq(PacketPtr pkt) override {
             return;
         }
     };
 
   public:
     CpuSidePort cpu_side_port;
     MemSidePort mem_side_port;
 
     EventFunctionWrapper memSendEvent;
     EventFunctionWrapper cpuSendEvent;
     EventFunctionWrapper writebackEvent;
     EventFunctionWrapper unblockEvent;
 
     PacketPtr toMem;
     PacketPtr toCPU;
     PacketPtr toWriteback;
 
     uint64_t latency;
     int assoc;
 
     bool blocked;
     bool writingback;
 
     PacketPtr pending;
 
     struct Block
     {
         uint8_t data[64];
         Addr tag;
         bool dirty;
         bool valid;
        uint64_t last_access_time;
     };
 
     // TODO: Your additional implementation (Class Variables) Here!
     Block *blks;
 
   protected:
     struct MicroCacheStats : public statistics::Group
     {
         MicroCacheStats(statistics::Group *parent);
         statistics::Scalar hits;
         statistics::Scalar misses;
         statistics::Formula hitRate;
     } stats;
 
 
 
   public:
 
     ///////////////////////////////////////////////////////
     // -------------FUNCTION-DECLARATIONS--------------- //
     ///////////////////////////////////////////////////////
 
     MicroCache(const MicroCacheParams *p);
     Port &getPort(const std::string &if_name, PortID idx);
 
     bool handleRequest(PacketPtr pkt);
     void handleResponse(PacketPtr pkt);
 
     /* Helper methods for callback (no need to call these directly) */
     void sendToMem() {
         assert (toMem != nullptr);
         mem_side_port.sendPacket(toMem);
         toMem = nullptr;
     }
 
     void writebackToMem() {
         assert (toWriteback != nullptr);
         mem_side_port.sendPacket(toWriteback);
         toWriteback = nullptr;
     }
 
     /***
      * Helper methods for communicating with upper/lower levels of hierarchy 
      ***/
 
     /* writeback data to lower levels */
     void writebackData(bool dirty, uint64_t addr, uint8_t *data) {
         assert(toWriteback == nullptr);
 
         RequestPtr req = std::make_shared<Request>(addr, 64, 0, 0);
 
         if (dirty) {
             // don't use MemCmd::WritebackDirty, because we want to get a write response
             // so we can block until write is complete
             toWriteback = new Packet(req, MemCmd::WriteReq, 64);
             toWriteback->allocate();
             if (toWriteback->getPtr<uint8_t>() != data) {
                 toWriteback->setData(data);
             }
         } else
             toWriteback = new Packet(req, MemCmd::WritebackClean, 64);
 
         schedule(writebackEvent, curTick() + latency * 100);
     };
 
     void unblock() {
         assert(blocked);
         assert(pending == nullptr);
         blocked = false;
         if (cpu_side_port.needRetry) {
             cpu_side_port.needRetry = false;
             cpu_side_port.sendRetryReq();
         }
     }
     
     /* send toCPU packet to higher levels of hierarchy */
     void sendToCpu() {
         if (toCPU->req->isCondSwap() || toCPU->isLLSC()) toCPU->req->setExtraData(0);
 
         assert(cpu_side_port.sendTimingResp(toCPU));
         toCPU = nullptr;
         unblock();
     }
 
     /* Request data at block addr from lower levels of hierarchy */
     void requestFromMem(uint64_t addr) {
         RequestPtr req = std::make_shared<Request>(addr, 64, 0, 0);
         PacketPtr memPkt = new Packet(req, MemCmd::ReadReq, 64);
         memPkt->allocate();
 
         // send request to memory
         mem_side_port.sendPacket(memPkt);
     }
 
     // TODO: Your Implementation (Helper Functions) Here!
 };
 
 
 
 }
 
 
 
 
 #endif // __MICRO_CACHE_HH__
 