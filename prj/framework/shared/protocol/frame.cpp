#include "shared/protocol/frame.hpp"
#include <algorithm>
#include <cstring>
namespace rail {
static void put16(uint8_t* p,uint16_t v){p[0]=uint8_t(v);p[1]=uint8_t(v>>8);} static void put32(uint8_t* p,uint32_t v){for(int i=0;i<4;i++)p[i]=uint8_t(v>>(8*i));}
static uint16_t get16(const uint8_t* p){return uint16_t(p[0]|(uint16_t(p[1])<<8));} static uint32_t get32(const uint8_t* p){uint32_t v=0;for(int i=0;i<4;i++)v|=uint32_t(p[i])<<(8*i);return v;}
uint32_t crc32(const uint8_t* data,size_t size){uint32_t c=0xffffffffu;for(size_t i=0;i<size;i++){c^=data[i];for(int j=0;j<8;j++)c=(c&1)?(c>>1)^0xedb88320u:c>>1;}return c^0xffffffffu;}
ErrorCode encode_frame(FrameView f, MutableByteView out,size_t& written){const size_t total=12+f.payload.size+4;if(f.payload.size>4096||out.data==nullptr||out.size<total){written=0;return ErrorCode::InvalidArgument;}uint8_t* p=out.data;p[0]=0x52;p[1]=0x49;p[2]=1;p[3]=0;put16(p+4,f.type);put32(p+6,f.sequence);put16(p+10,uint16_t(f.payload.size));if(f.payload.size)std::memcpy(p+12,f.payload.data,f.payload.size);put32(p+12+f.payload.size,crc32(p+2,10+f.payload.size));written=total;return ErrorCode::Ok;}
void FrameDecoder::feed(ByteView in,void* ctx,Callback cb){for(size_t i=0;i<in.size;i++){if(size_==MaxFrame){size_=0;protocol_errors_++;}buffer_[size_++]=in.data[i];while(size_>=12){if(buffer_[0]!=0x52||buffer_[1]!=0x49){std::memmove(buffer_,buffer_+1,--size_);continue;}if(buffer_[2]!=1){std::memmove(buffer_,buffer_+2,size_-=2);protocol_errors_++;continue;}const size_t total=12+get16(buffer_+10)+4;if(total>MaxFrame){std::memmove(buffer_,buffer_+2,size_-=2);protocol_errors_++;continue;}if(size_<total)break;const uint32_t want=get32(buffer_+12+get16(buffer_+10));const uint32_t got=crc32(buffer_+2,10+get16(buffer_+10));if(want!=got){crc_errors_++;std::memmove(buffer_,buffer_+2,size_-=2);continue;}if(cb)cb(ctx,{get16(buffer_+4),get32(buffer_+6),{buffer_+12,get16(buffer_+10)}});std::memmove(buffer_,buffer_+total,size_-=total);}}}
}
