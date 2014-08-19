#ifndef MESSAGE_H
#define MESSAGE_H

#define SIZE_MESSAGE 32

#define MESSAGE_NULL 0

struct Message
{
	int type;
	char data[SIZE_MESSAGE];
};

#endif
