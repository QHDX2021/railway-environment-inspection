#include "shared/recording/reader.hpp"
#include <iostream>
int main(int argc,char**argv){if(argc<3||std::string(argv[1])!="--input"){std::cerr<<"usage: replay_cli --input DATA_FILE [--recover]\n";return 2;}rail::ReadMode m=(argc>3&&std::string(argv[3])=="--recover")?rail::ReadMode::Recover:rail::ReadMode::Strict;rail::RecordReader r;auto x=r.read(argv[2],m,[](const rail::Record&){});std::cout<<"records="<<x.records<<" complete="<<x.complete<<" ok="<<x.ok<<"\n";return x.ok?0:1;}
