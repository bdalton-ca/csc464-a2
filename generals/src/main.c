#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define MAX_GENERALS 100
#define MAX_RECURSION 10

//#define TRAITOR_CHOICE RETREAT
#define TRAITOR_CHOICE ((rand()&1)? ATTACK:RETREAT)
enum
{
	NONE,
	ATTACK,
	RETREAT,
	N_COMMANDS
};

typedef struct GENERAL_t
{
	int id;
	int is_traitor;
	int votes[MAX_GENERALS][MAX_RECURSION];
	struct GENERAL_t * next;
	struct GENERAL_t * prev;
}GENERAL;

int command(GENERAL * generals, GENERAL * commander, int order, int n, int m)
{
	if(commander == generals) generals = generals->next;
	if(commander->next) commander->next->prev = commander->prev;
	if(commander->prev) commander->prev->next = commander->next;
	
	if(m==0)
	{
		for(GENERAL * g = generals; g; g = g->next)
		{
			if(commander->is_traitor||g->is_traitor)
				g->votes[commander->id][m] = TRAITOR_CHOICE;
			else
				g->votes[commander->id][m] = order;
		}
	}
	else
	{
		for(GENERAL * g = generals; g; g = g->next)
		{
			g->votes[commander->id][m-1] = commander->is_traitor? TRAITOR_CHOICE:order;
			command(generals,g,g->votes[commander->id][m-1],n,m-1);
		}
		
		for(GENERAL * g = generals; g; g = g->next)
		{
			if(g->is_traitor) g->votes[commander->id][m] = TRAITOR_CHOICE;
			else
			{				
				int votes[N_COMMANDS] = {0};
				for(GENERAL * g1 = generals; g1; g1 = g1->next)
					if(g!=g1) votes[g->votes[g1->id][m-1]]++;
				
				votes[g->votes[commander->id][m-1]]++;
				
				g->votes[commander->id][m] = votes[ATTACK]>votes[RETREAT]? ATTACK:RETREAT;
			}
		}
	}
	if(commander->next) commander->next->prev = commander;
	if(commander->prev) commander->prev->next = commander;
}

int main(int argc, char * argv[])
{
	if(argc < 5) 
	{
		printf
		(
			"Please supply the following arguments:\n"
				"\tn - number of generals\n"
				"\tt - number of traitors\n"
				"\tm - recursion depth\n"
				"\tc - 0 or 1, whether commander is a traitor\n"
		);
		return 0;
	}
	
	
	// THEOREM 1. For any m, Algorithm OM(m) 
	// satisfies conditions IC1 and IC2 if there are more than 
	// 3m generals and at most m traitors. 
	
	
	int n = atoi(argv[1]), t = atoi(argv[2]);
	int m = atoi(argv[3]), c = atoi(argv[4]);
	
	printf("testing with n=%d t=%d m=%d c=%d (No failures means test passes)\n",n,t,m,c);
	
	GENERAL generals[MAX_GENERALS] = {0};
	
	for(int i = 0; i < n; i++)
	{
		if(c && i==0)
			generals[i].is_traitor = 1;
		else if(i == 0)
			generals[i].is_traitor = 0;
		else
			generals[i].is_traitor = i<t? 1:0;
		
		generals[i].next = i!=n-1? &generals[ i+1 ] : NULL;
		generals[i].prev = i!= 0 ? &generals[ i-1 ] : NULL;
		generals[i].id = i;
	}
	
	srand(time(NULL));
	
	for(int i = 0; i < 100; i++)
	{
		int loyal_votes[N_COMMANDS]={0}, traitor_votes[N_COMMANDS]={0};
		int c_idx = 0;

		command(generals,&generals[c_idx],ATTACK,n,m);
			
		for(int j = 0; j<n; j++)
		if(j!=c_idx)
		{
			if(generals[j].is_traitor)
				traitor_votes[generals[j].votes[c_idx][m]]++;
			else
				loyal_votes[generals[j].votes[c_idx][m]]++;
		}
		//printf("L: a:%d r:%d\tT: a:%d r:%d\n", loyal_votes[ATTACK], loyal_votes[RETREAT], traitor_votes[ATTACK], traitor_votes[RETREAT]);
		
			
		//IC1. All loyal lieutenants obey the same order. 
		//IC2. If the commanding general is loyal, then every loyal lieutenant obeys the order he sends.
		if(loyal_votes[ATTACK]>0 && loyal_votes[RETREAT]>0)
		{
			printf("FAILED TO GET CONSENSUS (IC1)\n");
			break;
		}
		
		if(!generals[c_idx].is_traitor && loyal_votes[RETREAT]>0)
		{
			printf("FAILED TO ATTACK (IC2)\n");
			break;
		}
	}
	
	return 0;
}