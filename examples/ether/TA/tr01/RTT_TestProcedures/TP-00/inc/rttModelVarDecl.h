#ifndef _XXX_RTTMODELVARS_H_XXX_
#define _XXX_RTTMODELVARS_H_XXX_
typedef struct {

unsigned long long sut_End1I_t;
unsigned long long sut_End2I_t;
signed int sut_EtherI_processing;
unsigned long long sut_EtherI_t;
signed int sut_inmsg1a;
signed int sut_inmsg1p;
signed int sut_inmsg2a;
signed int sut_inmsg2p;
signed int sut_outmsg11a;
signed int sut_outmsg11p;
signed int sut_outmsg12a;
signed int sut_outmsg12p;
signed int sut_outmsg13a;
signed int sut_outmsg13p;
signed int sut_outmsg21a;
signed int sut_outmsg21p;
signed int sut_outmsg22a;
signed int sut_outmsg22p;
signed int sut_outmsg23a;
signed int sut_outmsg23p;
signed int te__tesimulator_l;
unsigned long long te__tesimulator_t;

} rttModelVars_t;

extern rttModelVars_t* rttStatePre;
extern rttModelVars_t* rttStatePost;

#endif
