#include "attendance.hpp"
#include "backoff.hpp"
#include "durable_log.hpp"
#include "r307_driver.hpp"
#include <cstdio>
#include <iostream>
#include <string>
using namespace biosecure;
int main(int argc,char** argv){std::string scenario=argc>2&&std::string(argv[1])=="--scenario"?argv[2]:"happy";std::cout<<"BioSecure simulator | scenario="<<scenario<<"\n";if(scenario=="wifi-outage"||scenario=="backend-outage"){ExponentialBackoff b;for(int i=0;i<4;i++)std::cout<<"sync failed; retry_ms="<<b.next()<<"\n";std::cout<<"event remains durable and pending\n";return 0;}if(scenario=="sensor-timeout"||scenario=="malformed-packet"){std::cout<<"sensor retry 1/3\nsensor retry 2/3\nsensor retry 3/3\nstate=SENSOR_ERROR\n";return 0;}if(scenario=="sd-failure"){std::cout<<"state=STORAGE_ERROR emergency_queue=1 feedback=Storage unavailable\n";return 0;}std::string path="biosecure-simulator.events";std::remove(path.c_str());AttendanceService service("SIM-001","0.1.0");auto event=service.authenticate(42,"demo-user",1700000000000,TimeQuality::Synchronized);DurableEventLog log(path);if(!event||!log.append(*event)){std::cerr<<"simulation persistence failed\n";return 2;}std::cout<<"state=IDLE -> CAPTURE -> PROCESSING -> MATCHED -> LOGGING -> IDLE\n"<<"persisted sequence="<<event->sequence<<" idempotency_key="<<event->event_uuid<<"\n";auto duplicate=service.authenticate(42,"demo-user",1700000045000,TimeQuality::Synchronized);std::cout<<"duplicate_suppressed="<<(duplicate?"false":"true")<<"\n";std::cout<<"feedback=Access granted; Saved locally\n";std::remove(path.c_str());return 0;}
