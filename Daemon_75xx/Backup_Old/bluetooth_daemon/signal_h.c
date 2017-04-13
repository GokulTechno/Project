#define _REENTRANT
#include <stdio.h>
#include <thread.h>
#include <signal.h>
#include <sys/types.h>

void *signal_hand(void *);

main()
{
sigset_t set;

/* block all signals in main thread.  Any other threads that are
   created after this will also block all signals */

sigfillset(&set); 

thr_sigsetmask(SIG_SETMASK, &set, NULL);

/* create a signal handler thread.  This thread will catch all
   signals and decide what to do with them.  This will only
   catch nondirected signals.  (I.e., if a thread causes a SIGFPE 
   then that thread will get that signal. */

thr_create(NULL, 0, signal_hand, 0, THR_NEW_LWP|THR_DAEMON|THR_DETACHED, NULL);

while (1) {
	/*
	Do your normal processing here....
	*/
	}  /* end of while */

return(0);
}

void *signal_hand(void *arg)
{
sigset_t set;
int sig;

sigfillset(&set); /* catch all signals */

while (1) {
 	/* wait for a signal to arrive */

	switch (sig=sigwait(&set)) {

	  /* here you would add whatever signal you needed to catch */
	  case SIGINT : {
			printf("Interrupted with signal %d, exiting...\n", sig);
			exit(0); 
			}

	  default : printf("GOT A SIGNAL = %d\n", sig); 
	  } /* end of switch */
	} /* end of while */

return((void *)0);
} /* end of signal_hand */
