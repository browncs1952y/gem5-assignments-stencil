

#include "mem/port.hh"
#include "params/EncryptionObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

/**
 * Example 2: An object that takes information from the CPU and sends it
 * to memory. Similarly, gets responses from memory and sends them to the
 * CPU. However, it encrypts/decrypts the packet data before sending!
 */
class EncryptionObject : public SimObject
{
  private:
    /**
     * Port that attaches to the CPU side (used for ICache and DCache).
     * Receive a packet, and give it to main object to handle the logic.
     */
     class CpuSidePort : public ResponsePort
     {
       private:
         // A reference to the main object
         EncryptionObject *owner;

         // True if we cannot handle CPU requests (due to high traffic)
         // We will let the CPU know when there is space, and it will
         // hold onto the packet until we notify it (and likely stall)
         bool needRetry;

      public:
         // If our packet send fails, then store it
         std::deque<PacketPtr> blockedPackets;

       public:
         CpuSidePort(const std::string& name, EncryptionObject *owner) :
             ResponsePort(name), owner(owner), needRetry(false)
         {  };

         /**
          * All response ports must override this function, just return 
          * whatever the memory device says.
          */
         AddrRangeList getAddrRanges() const override {
             return owner->mem_port.getAddrRanges();
         };

         void trySendRetry() {
             if (needRetry && blockedPackets.empty()) {
                 needRetry = false;
                 sendRetryReq();
             }
         };

         Tick recvAtomic(PacketPtr pkt) override {
             panic("Not handling atomic requests.");
             return -1;
         };

         /**
          * Receive a functional request packet from the CPU.
          *  These requests set the initial memory state prior to execution
          *  (e.g., to load the binary into simulated memory from the command)
          */
         void recvFunctional(PacketPtr pkt) override {
             owner->handleRequest(pkt, false);
         };

         /**
          * Receive a timing packet from the CPU.
          * These requests are from the true simulation state. Returns true
          * if we can handle the packet, otherwise the CPU will hold the
          * packet until we are ready.
          */
         bool recvTimingReq(PacketPtr pkt) override {
             if (owner->handleRequest(pkt)) {
                 return true;
             }

             needRetry = true;
             return false;
         };

         void recvRespRetry() override {
             assert(!blockedPackets.empty());

             // Try sending packets as long as we can and have some to send.
             while (!blockedPackets.empty()) {
                 if (sendTimingResp(blockedPackets.front())) {
                     blockedPackets.pop_front();
                 } else {
                     break;
                 }
             }
         };
     };

     class MemSidePort : public RequestPort
     {
       private:
         // A reference to the main object
         EncryptionObject *owner;

         // True if we cannot handle CPU requests (due to high traffic)
         // We will let the CPU know when there is space, and it will
         // hold onto the packet until we notify it (and likely stall)
         bool needRetry;

      public:
         // If our packet send fails, then store it
         std::deque<PacketPtr> blockedPackets;

       public:
         MemSidePort(const::std::string& name, EncryptionObject *owner) :
             RequestPort(name), owner(owner)
         {  };

       protected:
         /**
          * Receive a timing packet respose from memory.
          */
         bool recvTimingResp(PacketPtr pkt) override {
             if (owner->handleResponse(pkt)) {
                 return true;
             }

             needRetry = true;
             return false;
         };

         /**
          * If the memory side couldn't handle a packet that we sent,
          * then it'll tell us when it has space. Retry sending the
          * packet here.
          */
         void recvReqRetry() override {
             // We should have a blocked packet if this function is called.
             assert(!blockedPackets.empty());

             // Try sending packets as long as we can and have some to send.
             while (!blockedPackets.empty()) {
                 if (sendTimingReq(blockedPackets.front())) {
                     blockedPackets.pop_front();
                 } else {
                     break;
                 }
             }
         };
     };

  private:
    class CpuSidePort;
    class MemSidePort;

    CpuSidePort icache_port;
    CpuSidePort dcache_port;
    MemSidePort mem_port;

    uint8_t encryption_seed;
    std::vector<Addr> encrypted_addrs;

  public:
    /** Constructor */
    EncryptionObject(const EncryptionObjectParams &params);

    /**
     * All SimObjects need to implement a getPort function for the front end
     * to know which attribute of the class to use as the port.
     * Takes a port name (string) as input and returns the port associated
     * with that name.
     */
    Port &getPort(const std::string &if_name, PortID idx=InvalidPortID) override;

    /**
     * These functions handle packets from CPU and memory respectively.
     * They should return true if they can process the request issued by
     * the calling device (which should always be true).
     */
    bool handleRequest(PacketPtr pkt, bool timing = true);
    bool handleResponse(PacketPtr pkt);
};

};

