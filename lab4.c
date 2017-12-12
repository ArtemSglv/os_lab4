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


pthread_mutex_t stand_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t judge_in_the_hall_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t citi_in_hall_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t swearing_immi_mutex = PTHREAD_MUTEX_INITIALIZER;

int immi_in_hall, citi_in_hall, immi_in_stand, sweared_immi, max_stand_counter;

int immi_near_door, citi_near_door;

int judge_is_in_hall;


pthread_t immi_threads[IMINUM];
pthread_t citi_threads[CITINUM];
pthread_t judge_thread;

sem_t sem;

int limited_random(int max);

void *judge();
void *citizen();
void *immigrant();

int main()
{
	immi_in_hall = citi_in_hall = immi_in_stand = judge_is_in_hall = sweared_immi = max_stand_counter = immi_near_door = citi_near_door= 0;
	int i = 0;

	printf("Введите кол-во стоек.\n");
	scanf("%d", &max_stand_counter);
	printf("Кол-во стоек: %d\n", max_stand_counter);

	sem_init(&sem, 0, max_stand_counter);

	for(i=0; i<IMINUM; i++)
	{
		pthread_create(&immi_threads[i], NULL, immigrant, NULL);
	}

	for(i=0; i<CITINUM; i++)
	{
		pthread_create(&citi_threads[i], NULL, citizen, NULL);
	}

	pthread_create(&judge_thread, NULL, judge, NULL);
	
	while(1);
  return 0;
}

void *judge()
{
	while(1)
	{
		sleep(limited_random(30));

		//judge is trying to come to hall
		printf("Судья хочет войти в зал.\n");
		while(immi_in_stand != 0);//wait till there will be no immigrants

		pthread_mutex_lock(&judge_in_the_hall_mutex);
		
		judge_is_in_hall=1;
		pthread_mutex_lock(&citi_in_hall_mutex);
		printf("Судья в зале. %d иммгрантов в зале, %d граждан в зале.", immi_in_hall, citi_in_hall);
		printf("В очереди: %d иммигрантов, %d граждан.\n", immi_near_door, citi_near_door);
    	pthread_mutex_unlock(&citi_in_hall_mutex);
    	sleep(limited_random(15));

    	//wait till all immigrants swear
    	while(sweared_immi!=immi_in_hall);

    	pthread_mutex_lock(&swearing_immi_mutex);
    	sweared_immi  = 0;
    	pthread_mutex_unlock(&swearing_immi_mutex);

    	printf("Судья у выхода.\n");

    	judge_is_in_hall = 0;

    	printf("Судья ушел.\n");
    	pthread_mutex_unlock(&judge_in_the_hall_mutex);		
    	

    	
    }
}

void *citizen()
{
	int ci_in_hall=0;

	while(1)
	{
		sleep(limited_random(30));

		printf("Гражданин %ld хочет войти в зал.\n", syscall(SYS_gettid));
		citi_near_door++;
		do
		{
			if(!pthread_mutex_lock(&judge_in_the_hall_mutex))
			{
				pthread_mutex_lock(&citi_in_hall_mutex);
				citi_near_door--;
				citi_in_hall++;
				ci_in_hall = 1;
				printf("Гражданин %ld вошел в зал. %d иммигрантов в зале, %d граждан в зале. В очереди: %d иммигрантов, %d граждан.\n", syscall(SYS_gettid), immi_in_hall, citi_in_hall, immi_near_door, citi_near_door);
    			pthread_mutex_unlock(&citi_in_hall_mutex);
				pthread_mutex_unlock(&judge_in_the_hall_mutex);	
			}
			else { sleep(15); }
		} while (!ci_in_hall);

    	sleep(limited_random(30));

    	pthread_mutex_lock(&citi_in_hall_mutex);
		citi_in_hall--;
		printf("Гражданин %ld покинул зал.\n", syscall(SYS_gettid));
    	pthread_mutex_unlock(&citi_in_hall_mutex);
	}
}

void *immigrant()
{
	int im_in_the_hall = 0;

	sleep(limited_random(30));

	printf("Иммигрант %ld хочет войти в зал.\n", syscall(SYS_gettid));
	immi_near_door++;
	// enter immi in hall
	do{
	
		if(!pthread_mutex_trylock(&judge_in_the_hall_mutex))
		{
			if(immi_in_hall < MAXIMMINUM)
			{
				immi_near_door--;
				immi_in_hall++;
				im_in_the_hall = 1;
				pthread_mutex_lock(&citi_in_hall_mutex);
				printf("Иммигрант %ld вошел в зал. %d иммигрантов в зале, %d граждан в зале. В очереди: %d иммигрантов, %d граждан.\n", syscall(SYS_gettid), immi_in_hall, citi_in_hall, immi_near_door, citi_near_door);
    			pthread_mutex_unlock(&citi_in_hall_mutex);
    		}
			pthread_mutex_unlock(&judge_in_the_hall_mutex);
		}
		else { sleep(15); } 
	}while (!im_in_the_hall);
    
    //wait for the judge
	while(!judge_is_in_hall);
	
    //swear
    pthread_mutex_lock(&swearing_immi_mutex);
    sleep(limited_random(15)); //swearing...
    sweared_immi++;
	immi_in_stand++;
    printf("Иммигрант %ld принял гражданство. Иммигрантов для принятия гражданства: %d \n", syscall(SYS_gettid), immi_in_hall - sweared_immi);
    
    pthread_mutex_unlock(&swearing_immi_mutex);

    //go to stand as soon as judge leaves the hall

    //wait for the judge to leave
    while(judge_is_in_hall);

    sem_wait(&sem); // max_stand_counter--
	printf("Иммигрант %ld получил свою бумагу\n", syscall(SYS_gettid));
	sleep(limited_random(7));
    sem_post(&sem); // max_stand_counter++

	//exit
    pthread_mutex_lock(&judge_in_the_hall_mutex);
    immi_in_hall--;
    immi_in_stand--;
    printf("Иммигрант %ld покинул зал.\n", syscall(SYS_gettid));
    pthread_mutex_unlock(&judge_in_the_hall_mutex);

}


int limited_random(int max)
{
  return random() % max + 1;
}

