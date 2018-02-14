#ifndef _XXX_RTTINTERFACE_H_XXX_
#define _XXX_RTTINTERFACE_H_XXX_
typedef struct {

signed int in1a;
signed int in1p;
signed int in2a;
signed int in2p;
signed int out11a;
signed int out11p;
signed int out12a;
signed int out12p;
signed int out13a;
signed int out13p;
signed int out21a;
signed int out21p;
signed int out22a;
signed int out22p;
signed int out23a;
signed int out23p;

} rttInterface_t;

extern rttInterface_t* rttIOPre;
extern rttInterface_t* rttIOPost;

#endif
