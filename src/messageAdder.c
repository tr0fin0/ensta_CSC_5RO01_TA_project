#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h> 
#include <unistd.h>
#include <pthread.h>
#include "messageAdder.h"
#include "msg.h"
#include "iMessageAdder.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"



volatile unsigned int consume_count = 0;

pthread_t consumer;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

volatile MSG_BLOCK message_output;



/**
 * Increments the consume count.
 */
static void incrementConsumeCount(void);


/**
 * Consumer entry point.
 */
static void *sum(void *parameters);



MSG_BLOCK_ACCUMULATED getCurrentSum()
{
	pthread_mutex_lock(&mutex);
	MSG_BLOCK current_count = message_output;
	unsigned int accumulated_count = getConsumedCount();

	MSG_BLOCK_ACCUMULATED current_count_accumulated = {current_count, accumulated_count};
	pthread_mutex_unlock(&mutex);

    return current_count_accumulated;
}


static void incrementConsumeCount(void)
{
	consume_count++;
}


unsigned int getConsumedCount()
{
	return (unsigned int) consume_count;
}


void messageAdderInit(void)
{
	message_output.checksum = 0;
	for (size_t i = 0; i < DATA_SIZE; i++)
		message_output.mData[i] = 0;

    pthread_create(&consumer, NULL, sum, NULL);
}


void messageAdderJoin(void)
{
	pthread_join(consumer, NULL);
}


static void *sum(void *parameters)
{
	D(printf("[messageAdder] Thread created for sum with id %d\n", gettid()));
	unsigned int i = 0;

	while(i < ADDER_LOOP_LIMIT)
    {
		i++;
		sleep(ADDER_SLEEP_TIME);

        MSG_BLOCK message_new = getMessage();
		if (messageCheck(&message_new) == 0)
			printf("[messageAdder] Message corrupted\n");

		pthread_mutex_lock(&mutex);
		messageAdd(&message_output, &message_new);
		pthread_mutex_unlock(&mutex);

        incrementConsumeCount();
	}
	printf("[messageAdder] %d termination\n", gettid());

    pthread_exit(NULL);
}
