#include "timer.h"
#include <sys/time.h>   //timeval, gettimeofday


static struct timeval tstart;


void setTimer() {
	gettimeofday(&tstart,0);
}


unsigned long getTimer() {
	static struct timeval tend;
	gettimeofday(&tend,0);
	unsigned long millis = (tend.tv_usec - tstart.tv_usec)/1000;
	unsigned long sec = tend.tv_sec - tstart.tv_sec;
	if (millis<0) {
		millis += 1000;
		--sec;
	}
	return sec*1000+millis;
}
