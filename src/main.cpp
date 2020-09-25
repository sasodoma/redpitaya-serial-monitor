#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "main.h"


CStringParameter data ("data", CBaseParameter::RW, "", 0);



const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya serial monitor.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading serial monitor\n");
    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading serial monitor\n");
    return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}


int rp_get_params(rp_app_params_t **p)
{
    return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}








void UpdateSignals(void){}


void UpdateParams(void){}


void OnNewParams(void) {
	data.Update();
	fprintf(stdout, data.Value());
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}