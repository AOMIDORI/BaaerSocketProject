#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>

typedef struct{
	char *name;
	int status; // 0: offline,  1: online
}User;


void message_handler(int option){

}
//void follow_handler(int option){}
//void print_timeline(char *username){}
#endif
