#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include <pthread.h>
#include <semaphore.h>

#define MAX_MESSAGES 256
#define N_USERS 10

#include <windows.h>

sem_t pfmutex;
sem_t rnmutex;

//
//		Vector Clocks
//

void clock_send(unsigned src, unsigned * src_clock, unsigned * dst_clock)
{
	src_clock[src]++;
	if(dst_clock) memcpy(dst_clock,src_clock,sizeof(unsigned)*N_USERS);
}
void clock_receive(unsigned src, unsigned dst, unsigned * src_clock, unsigned * dst_clock)
{		
	dst_clock[dst]++;
	for(int i = 0; i < N_USERS; i++)
		dst_clock[i] = dst_clock[i]>src_clock[i]? dst_clock[i]:src_clock[i];
}
int clock_happens_before(unsigned * src, unsigned * dst)
{
	for(int i = 0; i < N_USERS; i++)
		if(src[i]>dst[i]) return 0;
	return 1;
}

void clock_print(char * prefix, unsigned * clock)
{
	sem_wait(&pfmutex);
	printf("clock %s = ", prefix);
	for(int i = 0; i < N_USERS; i++)
		printf(":%d:",clock[i]);
	printf("\n");
	sem_post(&pfmutex);
}

//
//		A1 Chatbot implementation + Vector Clocks
//

//#define printf_p(...)
#define printf_p(...) do\
{\
	sem_wait(&pfmutex);\
	printf(__VA_ARGS__);\
	sem_post(&pfmutex);\
}while(0)\

unsigned rand32()
{
	unsigned x;
	#define RAND_SH(X) ((((unsigned)rand())&0xFF)<<X)
	sem_wait(&rnmutex);
	x = RAND_SH(0) | RAND_SH(8) | RAND_SH(16) | RAND_SH(24);
	sem_post(&rnmutex);
	return x;
}

#define W32
#ifdef W32
#include "windows.h"
void sleep_ms(int ms) { Sleep(ms); }
#else
	UNIMPLEMENTED;
#endif

typedef struct MESSAGE_t
{
	unsigned clock[N_USERS];
	unsigned src_uid;
	char *msg,*src,*dst;
	struct MESSAGE_t * next;
}MESSAGE;

typedef struct
{
	int n_messages;
	MESSAGE messages[MAX_MESSAGES];
	MESSAGE * last_message;
	MESSAGE * free_message;
	sem_t mutex;
} CHATBOT;

typedef struct USER_t
{
	char * name;
	unsigned uid;
	struct USER_t * next;
	int is_online;
}USER;

typedef struct 
{
	int n_users;
	int last_uid;
	USER users[N_USERS];
	USER * last_user;
	USER * free_user;
	sem_t mutex;
}CHANNEL;

int n_messages=0;

void chatbot_start(CHATBOT * bot)
{
	memset(bot,0,sizeof(CHATBOT));
	for(int i = 0; i < MAX_MESSAGES-1; i++)
		bot->messages[i].next = &bot->messages[i+1];
	bot->free_message = &bot->messages[0];
	sem_init(&bot->mutex,0,1);
}

void chatbot_login(CHATBOT * bot, USER * user, unsigned * clock)
{	
	sem_wait(&bot->mutex);
	MESSAGE * prev = NULL;
	for(MESSAGE * node = bot->last_message; node; )
	{
		if(strcmp(node->dst,user->name)==0)
		{			
			clock_receive(node->src_uid,user->uid,node->clock,clock);
			
			printf_p("Hello %s, %s left this message: \"%s\"\n",node->dst,node->src,node->msg);
			clock_print(user->name,clock);			
			
			n_messages++;
			//free(node->msg);
			MESSAGE * next = node->next;
			if(prev) prev->next = node->next;
			if(node==bot->last_message) bot->last_message = node->next;
			
			node->next = bot->free_message;
			bot->free_message = node;
			
			bot->n_messages--;
			node = next;
		}
		else
		{
			prev = node;
			node = node->next;
		}
	}
	sem_post(&bot->mutex);
}

void chatbot_leave_message(CHATBOT * bot, char * msg, unsigned * clock, char * src, unsigned uid, char * dst)
{
	if(!dst || !msg) return;
	
	printf_p("%s leaving message for %s\n",src,dst);
	
	sem_wait(&bot->mutex);
	
	if(bot->free_message)
	{	
		MESSAGE * node = bot->free_message;
		bot->free_message = bot->free_message->next;
		node->next = bot->last_message;
		bot->last_message = node;
		
		bot->n_messages++;
		
		//int n = strlen(msg);
		//char * buffer = (char*) malloc(n);
		//memcpy(buffer,msg,n);
		
		node->msg = msg;
		node->src = src;
		node->dst = dst;
		node->src_uid = uid;
		clock_send(uid,clock,node->clock);
	}
	else
	{
		printf_p("Unable to store message: Out of memory.");
	}
	
	sem_post(&bot->mutex);
}


void channel_start(CHANNEL * channel)
{
	memset(channel,0,sizeof(CHANNEL));
	for(int i = 0; i < N_USERS-1; i++)
		channel->users[i].next = &channel->users[i+1];
	channel->free_user = &channel->users[0];
	sem_init(&channel->mutex,0,1);
}

#define GET_NEW_UID -1
USER * channel_login(CHANNEL * channel, char * name, unsigned uid)
{
	USER * node = NULL;
	printf_p("%s logged in\n",name);
	sem_wait(&channel->mutex);
	if(channel->free_user)
	{
		node = channel->free_user;
		channel->free_user = channel->free_user->next;
		node->next = channel->last_user;
		channel->last_user = node;
		
		channel->n_users++;
		
		node->name = name;
		node->is_online = 1;
		if(uid==GET_NEW_UID)
			node->uid = channel->last_uid++;
		else
			node->uid = uid;
	}
	else
	{
		printf_p("Channel is full.\n");
	}
	sem_post(&channel->mutex);
	return node;
}

void channel_logout(CHANNEL * channel, char * name)
{
	printf_p("%s logged out\n",name);
	sem_wait(&channel->mutex);
	USER * prev = NULL;
	for(USER * node = channel->last_user; node; node = node->next)
	{
		if(strcmp(node->name,name)==0)
		{
			node->is_online = 0;
			if(prev) prev->next = node->next;
			if(node==channel->last_user) channel->last_user = node->next;
			
			node->next = channel->free_user;
			channel->free_user = node;
			
			channel->n_users--;
			break;
		}
		
		prev = node;
	}
	sem_post(&channel->mutex);
}

char * channel_get_username(CHANNEL * channel)
{
	char * name = NULL;
	sem_wait(&channel->mutex);
	
	int r = channel->n_users? rand32()%channel->n_users : 0;
	for(USER * node = channel->last_user; node && 1+r--; node = node->next)
		name = node->name;
	
	sem_post(&channel->mutex);
	return name;
}

CHATBOT chatbot;
CHANNEL channel;
void * user_thread(void * data)
{	
	unsigned clock[N_USERS];
	memset(clock,0,sizeof(clock));
	
	char * name = (char*) data;
	char * dst = NULL;
	
	char message[32];
	snprintf(message,32,"Hello from %s!\0",name);
	
	USER * user = channel_login(&channel,name,GET_NEW_UID);
	unsigned uid = user->uid;
	channel_logout(&channel,name);
	
	for(;;)
	{
		user = channel_login(&channel,name,uid);
		
		chatbot_login(&chatbot,user,clock);		
		sleep_ms(rand32()%200);
		
		chatbot_leave_message(&chatbot,message,clock,name,uid,dst);		
		
		channel_logout(&channel,name);
		
		dst = channel_get_username(&channel);
		sleep_ms(rand32()%200);
	}
}

int main(int argc, char * argv[])
{
	sem_init(&pfmutex,0,1);
	sem_init(&rnmutex,0,1);
	channel_start(&channel);
	chatbot_start(&chatbot);
	
	srand(time(NULL));
	char names[N_USERS][4];
	pthread_t threads[N_USERS];
	for(int i = 0; i < N_USERS; i++)
	{
		for(int j = 0; j < 3; j++) names[i][j] = 'a'+rand32()%26;
		names[i][3] = '\0';
		pthread_create(&threads[i], NULL, user_thread, names[i]);
	}
	//sleep(1.0f*1000.0f);
	
	//printf("n_messages sent: %d\n",n_messages);
	
	for(;;);
	
	
	
	return 0;
}