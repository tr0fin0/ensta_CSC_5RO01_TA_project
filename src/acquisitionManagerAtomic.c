#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdatomic.h>
#include "acquisitionManager.h"
#include "msg.h"
#include "iSensor.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"



#define SEMAPHORE_FULL_NAME "/full"
#define SEMAPHORE_EMPTY_NAME "/empty"
#define SEMAPHORE_CHECK(sem)    \
	if (sem != SEM_FAILED)      \
		return ERROR_SUCCESS;   \
                                \
        perror("[sem_open");    \
                                \
        return ERROR_INIT;



_Atomic volatile unsigned int produce_count = 0;

pthread_t producers[4];

sem_t *semaphore_empty;
sem_t *semaphore_full;

pthread_mutex_t mutex_write = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_index = PTHREAD_MUTEX_INITIALIZER;

MSG_BLOCK buffer_data[256];
_Atomic volatile unsigned int buffer_data_read = 0;
_Atomic volatile unsigned int buffer_data_write = 0;

int buffer_index[256];
_Atomic volatile unsigned int buffer_index_read = 0;
_Atomic volatile unsigned int buffer_index_write = 0;



/*
 * Creates the synchronization elements.
 * @return ERROR_SUCCESS if the init is ok, ERROR_INIT otherwise
 */
static unsigned int createSynchronizationObjects(void);


/*
 * Increments the produce count.
 */
static void incrementProducedCount(void);


static void *produce(void *params);



static unsigned int createSynchronizationObjects(void)
{
	// Initialize semaphores
	sem_unlink(SEMAPHORE_EMPTY_NAME);
	sem_unlink(SEMAPHORE_FULL_NAME);

	semaphore_empty = sem_open(SEMAPHORE_EMPTY_NAME, O_CREAT, 0644, 255);
	semaphore_full = sem_open(SEMAPHORE_FULL_NAME, O_CREAT, 0644, 0);

	SEMAPHORE_CHECK(semaphore_empty);
	SEMAPHORE_CHECK(semaphore_full);

    printf("[acquisitionManager] Semaphore created\n");

	return ERROR_SUCCESS;
}


static void incrementProducedCount(void)
{
	produce_count++;
}


unsigned int getProducedCount(void)
{
	unsigned int p = 0;
	p = produce_count;

	return p;
}


MSG_BLOCK getMessage(void)
{
	sem_wait(semaphore_full);

    int index_local = buffer_index[buffer_index_read];

    buffer_index_read = (buffer_index_read + 1) % 256;
	MSG_BLOCK message = buffer_data[index_local];

    sem_post(semaphore_empty);

    return message;
}


void writeMessage(MSG_BLOCK message)
{
	sem_wait(semaphore_empty);

	pthread_mutex_lock(&mutex_write);
    int index_local = buffer_index_write;
	buffer_index_write = (buffer_index_write + 1) % 256;
    pthread_mutex_unlock(&mutex_write);

	buffer_data[index_local] = message;

	pthread_mutex_lock(&mutex_index);
	buffer_index[buffer_index_write] = index_local;
	buffer_data_write = (buffer_data_write + 1) % 256;
	pthread_mutex_unlock(&mutex_index);

    sem_post(semaphore_full);
}


unsigned int acquisitionManagerInit(void)
{
	unsigned int i;
	printf("[acquisitionManager] Synchronization initialization in progress...\n");
	fflush(stdout);

	if (createSynchronizationObjects() == ERROR_INIT)
		return ERROR_INIT;
	printf("[acquisitionManager] Synchronization initialization done.\n");

	for (i = 0; i < PRODUCER_COUNT; i++)
		pthread_create(&producers[i], NULL, produce, (void *)i);

	return ERROR_SUCCESS;
}

void acquisitionManagerJoin(void)
{
	unsigned int i;
	for (i = 0; i < PRODUCER_COUNT; i++)
		pthread_join(producers[i], NULL);

	sem_destroy(semaphore_empty);
	sem_destroy(semaphore_full);

	printf("[acquisitionManager] Semaphore cleaned\n");
}

void *produce(void *params)
{
	D(printf("[acquisitionManager] Producer created with id %d\n", gettid()));
	unsigned int i = 0;
	unsigned int index_producer = (unsigned int)params;

	while (i < PRODUCER_LOOP_LIMIT)
	{
		i++;
		sleep(PRODUCER_SLEEP_TIME + (rand() % 5));
		MSG_BLOCK message_block;

		getInput(index_producer, &message_block);
		if (messageCheck(&message_block) == 0)
			printf("[acquisitionManager] Message corrupted\n");

        writeMessage(message_block);
		incrementProducedCount();
	}

	printf("[acquisitionManager] %d termination\n", gettid());
	pthread_exit(NULL);
}
