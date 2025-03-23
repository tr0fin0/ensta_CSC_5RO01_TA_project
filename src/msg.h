#ifndef MSG_H
#define MSG_H



#define DATA_SIZE 256


typedef struct MSG_BLOCK_TAG
{
	unsigned int checksum;
	unsigned int mData[DATA_SIZE];
} MSG_BLOCK;



typedef struct MSG_BLOCK_TAG_ACCUMULATED
{
	MSG_BLOCK message_block;
	unsigned int consumed_count;
} MSG_BLOCK_ACCUMULATED;



/**
* Displays the message content
* @param message_block the message pointer
* @return 1 if the checksum is ok, 0 otherwise
*/
unsigned int messageCheck(volatile MSG_BLOCK* message_block);


/**
* Adds to the src message the content of add message
* @param src the message pointer
* @param add the message to add
*/
void messageAdd(volatile MSG_BLOCK* src, volatile MSG_BLOCK* add);

#endif
