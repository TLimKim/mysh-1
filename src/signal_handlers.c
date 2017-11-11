#include "signal_handlers.h"
#include "signal.h"
#include "unistd.h"
#include "stdio.h"

void catch_sigint(int signalNo)
{
    fflush(stdout);
    signal(signalNo, SIG_IGN);
    //fflush(stdout);
    
    flag_sig = 1;
}

void catch_sigtstp(int signalNo)
{
    fflush(stdout);
    signal(signalNo, SIG_IGN);
    //fflush(stdout);

    flag_sig = 1;
}
