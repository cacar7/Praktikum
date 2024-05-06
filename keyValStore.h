#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

void initializeStore();
int put(char * key, char * value);
int get(char * key, char * res);
int del(char * key);
void cleanupStore();

#endif

//Mier war hier :)