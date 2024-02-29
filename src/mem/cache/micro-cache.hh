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

namespace gem5 {

struct MicroCacheParams;

class MicroCache : public SimObject {
  private:
    class CpuSidePort : public ResponsePort {
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
            return ret;
        };

        // un-used for assignment, do not modify!
        Tick recvAtomic(PacketPtr pkt) override {
            return owner->mem_side_port.sendAtomic(pkt);
        }

        void recvRespRetry() override {  };

        void recvFunctional(PacketPtr pkt) override {
            owner->mem_side_port.sendFunctional(pkt);
        };

        AddrRangeList getAddrRanges() const override {
            return owner->mem_side_port.getAddrRanges();
        };

    };

    class MemSidePort : public RequestPort {
      private:
        MicroCache *owner;

      public:
        PacketPtr blocked_pkt;

        MemSidePort(const std::string &name, MicroCache *owner) :
            RequestPort(name), owner(owner), blocked_pkt(nullptr)
        {  };

      protected:
        bool recvTimingResp(PacketPtr pkt) override {
            owner->handleResponse(pkt);
            return true;
        };

        void recvReqRetry() override {
            if (blocked_pkt && sendTimingReq(blocked_pkt)) {
                blocked_pkt = nullptr;
            }
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

    bool blocked;

    struct Block
    {
        uint8_t data[64];
        Addr tags;
        bool dirty;
        bool valid;
    };

    PacketPtr pending;
    Block *blks;

    PacketPtr to_mem;
    PacketPtr to_cpu;
    PacketPtr to_writeback;

    uint64_t latency;

  protected:
    struct MicroCacheStats : public statistics::Group
    {
        MicroCacheStats(statistics::Group *parent);
        statistics::Scalar hits;
        statistics::Scalar misses;
	statistics::Formula hitRate;
    } stats;



  public:

    // TODO: Your Implementation (Class Variables) Here!


    ///////////////////////////////////////////////////////
    // -------------FUNCTION-DECLARATIONS--------------- //
    ///////////////////////////////////////////////////////

    MicroCache(const MicroCacheParams *p);
    Port &getPort(const std::string &if_name, PortID idx);

    bool handleRequest(PacketPtr pkt);
    void handleResponse(PacketPtr pkt);

    void writebackData(bool dirty, uint64_t addr, uint8_t *data) {
        assert(to_writeback == nullptr);

        RequestPtr req = std::make_shared<Request>(addr, 64, 0, 0);

        if (dirty) {
            to_writeback = new Packet(req, MemCmd::WritebackDirty, 64);
            to_writeback->allocate();
            if (to_writeback->getPtr<uint8_t>() != data) {
                to_writeback->setData(data);
            }
        } else
            to_writeback = new Packet(req, MemCmd::WritebackClean, 64);

        schedule(writebackEvent, curTick());
    };

    void sendToMem() {
        mem_side_port.sendTimingReq(to_mem);
        to_mem = nullptr;
    }

    void sendToCpu() {
        if (to_cpu->req->isCondSwap() || to_cpu->isLLSC()) to_cpu->req->setExtraData(0);

        assert(cpu_side_port.sendTimingResp(to_cpu));
        to_cpu = nullptr;

        assert(blocked);
        blocked = false;

        if (cpu_side_port.needRetry)
            cpu_side_port.sendRetryReq();

        cpu_side_port.needRetry = false;
    }

    void writebackToMem() {
        mem_side_port.sendTimingReq(to_writeback);
        to_writeback = nullptr;
    }


    // TODO: Your Implementation (Helper Functions) Here!
};



}




#endif // __MICRO_CACHE_HH__
