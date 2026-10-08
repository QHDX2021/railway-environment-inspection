#include "adapters/simulation/sample_source.hpp"
#include "linux/storage/task_writer.hpp"
#include <iostream>
int main(int argc,char**argv){if(argc!=3||std::string(argv[1])!="--output"){std::cerr<<"usage: capture_demo --output DIR\n";return 2;}rail::TaskWriter w;auto s=w.create(argv[2],{"demo",true});if(!s.ok()){std::cerr<<s.message<<"\n";return 1;}rail::SimulationSource src(100);auto r=rail::Pipeline().run(src,w,{});std::cout<<"produced="<<r.produced<<" saved="<<r.saved<<" ok="<<r.ok<<"\n";return r.ok?0:1;}
