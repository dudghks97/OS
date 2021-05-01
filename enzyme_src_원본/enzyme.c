#include "enzyme.h"

int please_quit;
int use_yield;
int workperformed;

// The code each enzyme executes.
void *run_enzyme(void *data) {
	
	thread_info_t *info;
	
	char *s = info->string;
	info->swapcount = 0;
	int oldvalue;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldvalue);
	if (s[0] =='C') pthread_cancel(pthread_self());
	 
	
	while(!please_quit) {	
	
	    if (s[0]>s[1]) {
		 workperformed=1;
		info->swapcount++;
		char tmp = s[0];
		s[0] = s[1];
		s[1] = tmp;
	    }
	    sched_yield();
	}
	return (void *)info;
}



// Make threads to sort string.
// Returns the number of threads created.
// There is a memory bug in this function.
int make_enzyme_threads(pthread_t * enzymes, char *string, void *(*fp)(void *)) {
	int i,rv,len;
	thread_info_t *info;
	len = strlen(string);
	info = (thread_info_t *)malloc(sizeof(thread_info_t));

	for(i=0;i<len-1;i++) {
	    info->string = string+i;
	    rv = pthread_create(enzymes+i,NULL,fp,info);
	    if (rv) {
	        fprintf(stderr,"Could not create thread %d : %s\n",
			i,strerror(rv));
		exit(1);
	    }
	}  
	return len-1;
}




// Join all threads at the end.
// Returns the total number of swaps.
int join_on_enzymes(pthread_t *threads, int n) {
	int i;
	int totalswapcount = 0;
	int whatgoeshere=0; // just to make the code compile 
	                    // you will need to edit the code below
	for(i=0;i<n;i++) {
	    void *status;
	    int rv = pthread_join(threads[i],&status);

        if(whatgoeshere) {
	    fprintf(stderr,"Can't join thread %d:%s.\n",i,strerror(rv));
	    continue;
	}

	if ((void*)whatgoeshere == PTHREAD_CANCELED) {
	    continue;
	} else if (status == NULL) {
	    printf("Thread %d did not return anything\n",i);
	    } else {
	      printf("Thread %d exited normally: ",i);// Don't change this line
	      int threadswapcount = whatgoeshere; 
	      // Hint - you will need to cast something.
	      printf("%d swaps.\n",threadswapcount); // Don't change this line
	      totalswapcount += threadswapcount;// Don't change this line
	    }
	}	
	return totalswapcount;
}

/* Wait until the string is in order. Note, we need the workperformed flag just in case a thread is in the middle of swapping characters
so that the string temporarily is in order because the swap is not complete.
*/
void wait_till_done(char *string, int n) {
	int i;
	while(1) {
	    sched_yield();
	    workperformed=0;
	    for(i=0;i<n;i++) 
	        if (string[i] > string[i+1]) {
	            workperformed=1;
	    	}
	    if(workperformed==0) break;
	}
}

void * sleeper_func(void *p) {
	sleep( (int) p); 
	// Actually this may return before p seconds because of signals. 
	// See man sleep for more information
	printf("sleeper func woke up - exiting the program\n");
	exit(1);
}

int smp2_main(int argc, char **argv) {
	pthread_t enzymes[MAX];
	int n,totalswap;
	char string[MAX];

	if (argc <= 1) {
	    fprintf(stderr,"Usage: %s <word>\n",argv[0]);
	    exit(1);
	}
	strncpy(string,argv[1],MAX); // Why is this necessary? Why cant we give argv[1] directly to the thread functions?

	please_quit = 0;
	use_yield =1;
	
	printf("Creating threads...\n");
	n = make_enzyme_threads(enzymes,string,run_enzyme);
	printf("Done creating %d threads.\n",n);
	
	pthread_t sleeperid;
	pthread_create(&sleeperid,NULL,sleeper_func,(void*)5);

	wait_till_done(string,n);
	please_quit = 1;
	printf("Joining threads...\n");
	totalswap = join_on_enzymes(enzymes, n);
	printf("Total: %d swaps\n",totalswap);
	printf("Sorted string: %s\n",string);
	
	exit(0);
}



