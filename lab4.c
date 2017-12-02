#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <semaphore.h>

#define IMINUM 10
#define CITINUM 2

#define MAXIMMINUM 4
#define MAXSTANDCOUNTER 2


//pthread_mutex_t stand_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t judge_in_in_the_hall_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t citi_in_hall_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t swearing_immi_mutex = PTHREAD_MUTEX_INITIALIZER;

int immi_in_hall, citi_in_hall, immi_in_stand, sweared_immi, max_stand_counter;

int judge_is_in_da_town;

typedef void* (*thread_func) (void* arg);

pthread_t immi_threads[IMINUM];
pthread_t citi_threads[CITINUM];
pthread_t judge_thread;

sem_t sem;

int limited_random(int max);

void judge();
void citizen();
void immigrant();

int main()
{
	immi_in_hall = citi_in_hall = immi_in_stand = judge_is_in_da_town = sweared_immi = max_stand_counter = 0;
	int i = 0;

	printf("Enter max stand quantity.\n");
	scanf("%d", &max_stand_counter);
	printf("It is %d\n", max_stand_counter);

	sem_init(&sem, 0, max_stand_counter);

	for(i=0; i<IMINUM; i++)
	{
		pthread_create(&immi_threads[i], NULL, (thread_func)immigrant, NULL);
	}

	for(i=0; i<CITINUM; i++)
	{
		pthread_create(&citi_threads[i], NULL, (thread_func)citizen, NULL);
	}

	pthread_create(&judge_thread, NULL, (thread_func)judge, NULL);
	
	while(1);
  return 0;
}

void judge()
{
	while(1)
	{
		sleep(limited_random(30));

		//judge is trying to come to hall
		printf("Judge wanna enter the hall.\n");
		while(immi_in_stand != 0);//wait till there will be no immigrants

		pthread_mutex_lock(&judge_in_in_the_hall_mutex);
		
		judge_is_in_da_town=1;
		pthread_mutex_lock(&citi_in_hall_mutex);
		printf("Judge is in the hall.\n %d immigrants in the hall, %d citizens in the hall.\n", immi_in_hall, citi_in_hall);
    	pthread_mutex_unlock(&citi_in_hall_mutex);
    	sleep(limited_random(15));

    	//wait till all immigrants swear
    	while(sweared_immi!=immi_in_hall);

    	pthread_mutex_lock(&swearing_immi_mutex);
    	sweared_immi  = 0;
    	pthread_mutex_unlock(&swearing_immi_mutex);

    	printf("Judge is about to leave.\n");

    	judge_is_in_da_town = 0;

    	printf("Judge leaves the hall.\n");
    	pthread_mutex_unlock(&judge_in_in_the_hall_mutex);		
    	

    	
    }
}

void citizen()
{


	while(1)
	{//syscall(SYS_gettid)
		sleep(limited_random(30));

		printf("Citizen %ld wanna enter the hall.\n", syscall(SYS_gettid));

		pthread_mutex_lock(&judge_in_in_the_hall_mutex);
		pthread_mutex_lock(&citi_in_hall_mutex);
		citi_in_hall++;
		printf("Citizen %ld came to the hall.\n %d immigrants in the hall, %d citizens in the hall.\n", syscall(SYS_gettid), immi_in_hall, citi_in_hall);
    	pthread_mutex_unlock(&citi_in_hall_mutex);
    	pthread_mutex_unlock(&judge_in_in_the_hall_mutex);		

    	sleep(limited_random(30));

    	pthread_mutex_lock(&citi_in_hall_mutex);
		citi_in_hall--;
		printf("Citizen %ld left the hall.\n", syscall(SYS_gettid));
    	pthread_mutex_unlock(&citi_in_hall_mutex);		
	}
}

void immigrant()
{
	int im_in_the_hall = 0;

	long tid = syscall(SYS_gettid) - (long) 2800;

	sleep(limited_random(30));

	printf("Immigrant %ld wanna enter the hall.\n", syscall(SYS_gettid));

	do{
	
		pthread_mutex_lock(&judge_in_in_the_hall_mutex);
		if(immi_in_hall < MAXIMMINUM)
		{
			immi_in_hall++;
			im_in_the_hall = 1;
			pthread_mutex_lock(&citi_in_hall_mutex);
			printf("Immigrant %ld came to the hall.\n %d immigrants in the hall, %d citizens in the hall.\n", syscall(SYS_gettid), immi_in_hall, citi_in_hall);
    		pthread_mutex_unlock(&citi_in_hall_mutex);
    	}
    	pthread_mutex_unlock(&judge_in_in_the_hall_mutex);
	}while (!im_in_the_hall);
    
    //wait for the judge
    while(!judge_is_in_da_town);
    //swear
    pthread_mutex_lock(&swearing_immi_mutex);
    sleep(limited_random(15));
    sweared_immi++;
    immi_in_stand++;
    printf("Immigrant %ld sweared to be a good citizen. \nImmigrants to swear left: %d \n", syscall(SYS_gettid), immi_in_hall - sweared_immi);
    
    pthread_mutex_unlock(&swearing_immi_mutex);

    //go to stand as soon as judge leaves the hall

    //wait for the judge to leave
    while(judge_is_in_da_town);

    sem_wait(&sem);
	printf("Immigrant %ld is having his papers prepared.\n", syscall(SYS_gettid));
	sleep(limited_random(7));    
    sem_post(&sem);

    pthread_mutex_lock(&judge_in_in_the_hall_mutex);
    immi_in_hall--;
    immi_in_stand--;
    printf("Immigrant %ld left the hall with his papers.\n", syscall(SYS_gettid));
    pthread_mutex_unlock(&judge_in_in_the_hall_mutex);

}


int limited_random(int max)
{
  return random() % max + 1;
}

