#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "displayManager.h"
#include "iDisplay.h"
#include "iAcquisitionManager.h"
#include "iMessageAdder.h"
#include "msg.h"
#include "multitaskingAccumulator.h"
#include "debug.h"



pthread_t display_thread;



/**
 * Display manager entry point.
 * */
static void *display( void *parameters );



void displayManagerInit(void)
{
	pthread_create(&display_thread, NULL, display, NULL);
}


void displayManagerJoin(void)
{
	pthread_join(display_thread, NULL);
} 


static void *display(void *parameters)
{
	D(printf("[displayManager] Thread created for display with id %d\n", gettid()));//gettid()));
	unsigned int difference_count = 0;

	while(difference_count < DISPLAY_LOOP_LIMIT)
    {
		sleep(DISPLAY_SLEEP_TIME);
		MSG_BLOCK_ACCUMULATED accumulated_count = getCurrentSum();
		MSG_BLOCK current_count = accumulated_count.message_block;
		unsigned int consumed_count = accumulated_count.consumed_count;

        if (messageCheck(&current_count) == 0)
			printf("[displayManager] Message corrupted\n");

		messageDisplay(&current_count);

        print(getProducedCount(), consumed_count);
	}

    printf("[displayManager] %d termination\n", gettid());
    pthread_exit(NULL);
}
