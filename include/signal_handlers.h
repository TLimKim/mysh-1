#ifndef SIGNAL_H_
#define SIGNAL_H_

void catch_sigint(int signalNo);

void catch_sigtstp(int signalNo);

int flag_sig;

#endif // SIGNAL_H_
