//taskScheduler.h

typedef struct Task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
};
