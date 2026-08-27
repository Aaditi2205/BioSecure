#include "attendance.hpp"
#include "backoff.hpp"
#include "durable_log.hpp"
#include "r307_driver.hpp"
#include "r307_packet.hpp"
#include "state_machine.hpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace biosecure;using namespace biosecure::r307;
static int failures=0;
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__FILE__<<":"<<__LINE__<<" "#x"\n";++failures;}}while(0)
class FakeTransport:public IByteTransport{public:int writes=0;int timeouts=0;std::vector<std::uint8_t> response;bool write(const std::uint8_t*,std::size_t)override{++writes;return true;}bool readPacket(std::vector<std::uint8_t>& out,std::chrono::milliseconds)override{if(timeouts-->0)return false;out=response;return !out.empty();}};
int main(){
 Packet p{kDefaultAddress,PacketId::Command,{0x13,0,0,0,0}};auto wire=serialize(p);auto parsed=deserialize(wire);CHECK(parsed.error==ParseError::None);CHECK(parsed.packet.payload==p.payload);wire.back()^=1;CHECK(deserialize(wire).error==ParseError::BadChecksum);CHECK(deserialize({1,2}).error==ParseError::TooShort);auto malformed=serialize(p);malformed[8]=1;CHECK(deserialize(malformed).error==ParseError::InvalidLength);
 FakeTransport fake;fake.timeouts=2;fake.response=serialize(Packet{kDefaultAddress,PacketId::Ack,{0x00}});Driver driver(fake,kDefaultAddress,2,std::chrono::milliseconds(1));auto vr=driver.verifyPassword(0);CHECK(vr.ok());CHECK(vr.attempts==3);CHECK(fake.writes==3);FakeTransport always;always.timeouts=9;Driver limited(always,kDefaultAddress,1,std::chrono::milliseconds(1));auto fail=limited.captureImage();CHECK(fail.error==DriverError::Timeout);CHECK(fail.attempts==2);
 StateMachine sm;CHECK(!sm.transition(State::Matched));CHECK(sm.transition(State::SelfTest));CHECK(sm.transition(State::Idle));CHECK(sm.transition(State::Capture));CHECK(sm.transition(State::Processing));CHECK(sm.transition(State::Denied));CHECK(sm.transition(State::Lockout));CHECK(sm.transition(State::Idle));
 AttendanceService a("D","1",45000);auto e1=a.authenticate(7,"U",100000,TimeQuality::Unknown);CHECK(e1.has_value());CHECK(!a.authenticate(7,"U",145000,TimeQuality::Unknown));auto e2=a.authenticate(7,"U",145001,TimeQuality::Estimated);CHECK(e2&&e2->sequence==2);CHECK(e1->event_uuid.size()==36);auto json=e1->canonicalJson();CHECK(json.find("fingerprint")==std::string::npos);CHECK(json.find("template_data")==std::string::npos);CHECK(json.find("sensor_template_id")!=std::string::npos);
 ExponentialBackoff b(100,500);CHECK(b.next()==100);CHECK(b.next()==200);CHECK(b.next()==400);CHECK(b.next()==500);b.reset();CHECK(b.next()==100);
 const char* path="biosecure-test.events";std::remove(path);DurableEventLog log(path);CHECK(log.append(*e1));CHECK(log.append(*e2));std::size_t bad=0;auto recovered=log.recover(&bad);CHECK(recovered.size()==2);CHECK(bad==0);{std::ofstream f(path,std::ios::app);f<<"truncated";}recovered=log.recover(&bad);CHECK(recovered.size()==2);CHECK(bad==1);std::remove(path);
 if(failures){std::cerr<<failures<<" failures\n";return 1;}std::cout<<"all host tests passed\n";return 0;
}
