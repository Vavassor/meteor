#ifndef THREAD_MESSAGES_H
#define THREAD_MESSAGES_H

#define MESSAGE_RESIZE	1
#define MESSAGE_MOUSE	2

struct ViewportData
{
	int width, height;
};

struct MouseData
{
	int delta[2];
};

#endif
