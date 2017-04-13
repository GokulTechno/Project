#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function1( void *ptr );
void *print_message_function2( void *ptr );

main()
{
     pthread_t thread1, thread2;
     int  iret1, iret2;

    /* Create independent threads each of which will execute function */

     iret1 = pthread_create( &thread1, NULL, print_message_function1, NULL);
     if(iret1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }

     iret2 = pthread_create( &thread2, NULL, print_message_function2, NULL);
     if(iret2)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
         exit(EXIT_FAILURE);
     }

     //printf("pthread_create() for thread 1 returns: %d\n",iret1);
     //printf("pthread_create() for thread 2 returns: %d\n",iret2);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 

     exit(EXIT_SUCCESS);
}

void *print_message_function1( void *ptr )
{
while(1)
{
     printf("message1\n");
	sleep(1);
}
}

void *print_message_function2( void *ptr )
{
while(1)
{
     printf("message2\n");
	sleep(1);
}
}

