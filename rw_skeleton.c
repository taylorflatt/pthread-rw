// gcc -Wall rw.c -o rw -pthread -I/home/taylor/C_Lab2/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <rw.h>
#include <unistd.h>
#include <ctype.h>

#define SLOWNESS 30000

#define INVALID_ACCNO -99999


// sleep function
void rest() {
    usleep(SLOWNESS * (rand() % (1)));
    //usleep(100);
}

/* Global shared data structure */
account account_list[SIZE]; 								/* this is the data structure that the readers and writers will be accessing concurrently.*/

pthread_mutex_t r_lock = PTHREAD_MUTEX_INITIALIZER;			/* read lock ,shared only between readers */
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;		/* read-write lock ,shared between readers and writers */
int read_count = 0;											/* keeps track of number of readers inside the CS */

/* Writer thread - will update the account_list data structure. 
   Takes as argument the seed for the srand() function.	
*/
void * writer_thr(void * arg) {
	printf("Writer thread ID %ld\n", pthread_self());	
	srand(time(NULL));		/* set random number seed for this writer */
	
	int i, j;
	int r_idx;
	unsigned char found;            /* For every update_acc[j], set to TRUE if found in account_list, else set to FALSE */
	account update_acc[WRITE_ITR];

	/* first create a random data set of account updates */
	for (i = 0; i < WRITE_ITR;i++) {
		r_idx = rand() % SIZE;		/* a random number in the range [0, SIZE) */ 
		update_acc[i].accno = account_list[r_idx].accno;
		update_acc[i].balance = 1000.0 + (float) (rand() % MAX_BALANCE);
	}
	/* open a writer thread log file */
	char thr_fname[64];
	snprintf(thr_fname, 64, "writer_%ld_thr.log", pthread_self());
	FILE* fd = fopen(thr_fname, "w");
	if (!fd) {
		fprintf(stderr,"Failed to open writer log file %s\n", thr_fname);
		pthread_exit(&errno);
	}

	/* The writer thread will now to update the shared account_list data structure. 
	   For each entry 'j' in the update_acc[] array, it will find the corresponding 
       account number in the account_list array and update the balance of that account
	   number with the value stored in update_acc[j]. 	
	*/
	
	int temp_accno;
	float temp_balance;
    /* The writer thread will now to update the shared account_list data structure */
	
	// This is the random data set. So this will traverse all of the accounts in update_acc.
    for (j = 0; j < WRITE_ITR;j++) {
        found = FALSE;
        /* Now update */
		
		// This is the account list. So this will traverse ALL of the accounts until it finds the one fitting our account.
        for (i = 0; i < SIZE;i++) 
		{
			
            if (account_list[i].accno == update_acc[j].accno) {
				
				/* lock and update location */
                /* You MUST FIRST TEMPORARILY INVALIDATE the accno by setting account_list[i] = INVALID_ACCNO; before making any updates to the account_list[i].balance.
				   Once the account balance is updated, you MUST put the rest() call in the appropriate place before going for update_acc[j+1]. 
				   This is to enable detecting race condition with reader threads violating CS entry criteria.
 				   For every successful update to the account_list, you must write a log entry using the following format string:
						 fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n",
                        				account_list[i].accno, update_acc[j].accno, account_list[i].balance, update_acc[j].balance);
				   
				   Additionally, your code must also introduce checks/test to detect possible corruption due to race condition from CS violations.	
				*/
				/* YOUR CODE FOR THE WRITER GOES IN HERE */
				
				pthread_mutex_lock(&rw_lock); // Acquire read/write lock.
				// Update location? //
				
				temp_accno = account_list[i].accno; // Store the account number.
				temp_balance = account_list[i].balance; // Store the account balance.
				
				account_list[i].accno = INVALID_ACCNO; // Temporarily invalidate the account number.
				
				account_list[i].balance = update_acc[j].balance; // Update the balance of the account.
				
				account_list[i].accno = temp_accno; // Set the account number back to its original number.
				
				fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n", account_list[i].accno, update_acc[j].accno, temp_balance, update_acc[j].balance);
				
				found = TRUE;
				
				rest();                 /* makes the write long duration - PLACE THIS IN THE CORRECT PLACE SO AS TO INTRODUCE LATENCY IN WRITE before going for next 'j' */
				
				pthread_mutex_unlock(&rw_lock);
			}
        }
		
        if (!found)
            fprintf(fd, "Failed to find account number %d!\n", update_acc[j].accno);

    }   // end test-set for-loop
	
	fclose(fd);

	return NULL;
}

/* Reader thread - will read the account_list data structure. 
   Takes as argument the seed for the srand() function. 
    * 
	* SIZE refers to the size of the TEST (read_acc) list.
	* READER_ITR refers to the size of the real (account_list) list.
*/
void * reader_thr(void *arg) {
	printf("Debug: Entered reader_thr method. \n");
	
	printf("Reader thread ID %ld\n", pthread_self());
	
	printf("Debug: Before srand.\n ");
    //srand(*((unsigned int *) arg));   /* set random number seed for this reader */
	srand(time(NULL));
	printf("Debug: After srand.\n ");
	
    int i, j;
	int r_idx;
	//int read_count = 0;				/* Keeps track of the number of readers inside the CS */
	unsigned char found;			/* For every read_acc[j], set to TRUE if found in account_list, else set to FALSE */
    account read_acc[READ_ITR];
	
	printf("Debug: After initializers.\n ");

    /* first create a random data set of account updates */
    for (i = 0; i < READ_ITR;i++) {
		//printf("Debug: Entered the first for-loop (i = %d).\n", i);
        r_idx = rand() % SIZE;      /* a random number in the range [0, SIZE) */
        read_acc[i].accno = account_list[r_idx].accno;
        read_acc[i].balance = 0.0;		/* we are going to read in the value */
    }
	
	printf("Debug: After First for-loop. \n");
    /* open a reader thread log file */
	char thr_fname[64];
	snprintf(thr_fname, 64, "reader_%ld_thr.log", pthread_self());
	FILE *fd = fopen(thr_fname, "w");
	if (!fd) {
		fprintf(stderr,"Failed to reader log file %s\n", thr_fname);
		pthread_exit(&errno);
	}
 
    /* The reader thread will now to read the shared account_list data structure.
	   For each entry 'j' in the read_acc[] array, the reader will fetch the 
       corresponding balance from the account_list[] array and store in
       read_acc[j].balance. The reader will then make a log entry in the log file
	   for every successful read from the account_list using the format: 
			fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n",
                        	account_list[i].accno, read_acc[j].accno, read_acc[j].balance);  
	   
	   If it is unable to find a account number, then the following log entry must be
	   made: fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);

	   Additionally, your code must also introduce checks/test to detect possible corruption due to race condition from CS violations.	

 	*/

	printf("Debug: Before second for-loop.\n ");
	
    /* The reader thread will now to read the shared account_list data structure */
	
	// This is the random data set. So this will traverse all of the accounts in read_acc.
    for (j = 0; j < READ_ITR; j++) 
	{
        /* Now read the shared data structure */
        found = FALSE;
		
		//printf("Debug: Entered second for-loop (j = %d).\n ", j);
		
		// This is the account list. So this will traverse ALL of the accounts until it finds the one fitting our account.
        for (i = 0; i < SIZE; i++) 
		{
			//printf("Debug: Entered third for-loop (i = %d). \n", i);
			rest();
			
			if (account_list[i].accno == read_acc[j].accno) 
			{
				found = TRUE;
				
				pthread_mutex_lock(&r_lock); // Lock the file against readers.
				read_count++;

				if(read_count == 1)
					pthread_mutex_lock(&rw_lock); // Lock the file against writers.

				pthread_mutex_unlock(&r_lock); // Unlock the reader's restriction.
				
				read_acc[j].balance = account_list[i].balance;

				fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n", account_list[i].accno, read_acc[j].accno, read_acc[j].balance);  

				/* Now that we are finished reading, we need to clean up */

				pthread_mutex_lock(&r_lock);

				read_count--;

				if (read_count == 0)
					pthread_mutex_unlock(&rw_lock); // Unlock the writer's restriction.

				pthread_mutex_unlock(&r_lock); // Unlock the reader's restriction.
			}
		}
		
		if (!found)
		{
			fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);
			found = FALSE;
		}
    }   // end test-set for-loop

    fclose(fd);
	
	printf("Debug: End of reader_thr method.\n ");
	
    return NULL;
}

/* populate the shared account_list data structure */
void create_testset() {
	time_t t;
	srand(time(&t));
	int i;	
	
	FILE *fd = fopen("test.log", "w");
	if (!fd) {
		fprintf(stderr,"Failed to test case log file \n");
		pthread_exit(&errno);
	}
	
	for (i = 0;i < SIZE;i++) {
		account_list[i].accno = 1000 + rand() % RAND_MAX;
		account_list[i].balance = 100 + rand() % MAX_BALANCE;
		
		fprintf(fd, "Account number = %d, balance = %6.2f\n", account_list[i].accno, account_list[i].balance);  
	}	
	
	fclose(fd);
	return;
}


/* Prints the using statement for invalid command line execution syntax */
void usage(char *str) {
	printf("Usage: %s -r <NUM_READERS> -w <NUM_WRITERS>\n", str);
	return;
}

/* Returns TRUE if str is a natural number (aka no negatives) or FALSE otherwise. */
int isInt(char *str)
{
    while (*str)
    {
        if (!isdigit(*str++)) 
           return FALSE;
    }
	
    return TRUE;
}


int main(int argc, char *argv[]) {
	time_t t;
	unsigned int seed;
	int i;
	void *result;
	int index;

	int READ_THREADS = 0;			/* number of readers to create */
	int WRITE_THREAD = 0;			/* number of writers to create */
	
	/* Generate a list of account informations. This will be used as the input to the Reader/Writer threads. */
	create_testset();
	
	int c;
	
	// There is at least a single argument, parse it.
	while ((c = getopt (argc, argv, "r:w:")) != -1)
	{
		switch (c)
		{
			case '?':
				usage(argv[0]);
				abort();
				
			case 'r':
			{
				printf("r = %d \n ", atoi(optarg));
				
				if(isInt(optarg) == TRUE)
				{
					READ_THREADS = atoi(optarg);
					break;
				}
				
				else
				{
					usage(argv[0]);
					abort();
				}
			}
				
			case 'w':
			{
				if(isInt(optarg) == TRUE)
				{
					WRITE_THREAD = atoi(optarg);
					break;
				}
				
				else
				{
					usage(argv[0]);
					abort();
				}
			}
			
			default:
				usage(argv[0]);
				abort();
		}
	}
	
	// For any case in which we don't have a -r and/or -w:
	for ( ; optind < argc; optind++) 
	{
		usage(argv[0]);
		abort();
	}
	
	pthread_t* reader_idx = (pthread_t *) malloc(sizeof(pthread_t) * READ_THREADS);		/* holds thread IDs of readers */
	pthread_t* writer_idx  = (pthread_t *) malloc(sizeof(pthread_t) * WRITE_THREAD);		/* holds thread IDs of writers */
	
	/* create readers */
  	for (i = 0; i < READ_THREADS; i++) {
		seed = (unsigned int) time(&t);
		
		// pthread_create returns a non-zero number if there was an error.
		if (pthread_create(reader_idx + i, NULL, reader_thr, (void *) (intptr_t) i) != 0) {
        perror("pthread create");
		printf("There was an error with the pthread_create method. ");
        exit(-1);
		}
	}
	
  	printf("Done creating reader threads!\n");

	/* create writers */ 
  	for (i = 0; i < WRITE_THREAD; i++) {
		seed = (unsigned int) time(&t);
		
		// pthread_create returns a non-zero number if there was an error. 
		if (pthread_create(writer_idx + i, NULL, writer_thr, (void *) (intptr_t) i) != 0) {
		  perror("pthread create");
		  exit(-1);
		}
	}
	
	printf("Done creating writer threads!\n");

  	/* Join all reader and writer threads.
    */
	
	 i = 0;
	 while (i < WRITE_THREAD) {
		pthread_join(writer_idx[i], &result);
		printf("Joined %d with status: %ld\n", i, (intptr_t) result);
		i++;
	}
	
	printf("Writer threads joined.\n");
	
	i = 0;
	while (i < READ_THREADS) {
		pthread_join(reader_idx[i], &result);
		printf("Joined %d with status: %ld\n", i, (intptr_t) result);
		i++;
	}

	printf("Reader threads joined.\n"); 
	
	/* clean-up - always a good thing to do! */
 	pthread_mutex_destroy(&r_lock);
	pthread_mutex_destroy(&rw_lock);
	
	return 0;
}
	

