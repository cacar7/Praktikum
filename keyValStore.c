#include "keyValStore.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KEYS 100
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

typedef struct {
    char key[256];
    char value[256];
} KeyValue;

static KeyValue *store;
static int shmid;
static int semid;

struct sembuf p = { 0, -1, SEM_UNDO}; // P-Operation
struct sembuf v = { 0, +1, SEM_UNDO}; // V-Operation

void lock() {
    if (semop(semid, &p, 1) == -1) {
        perror("semop lock");
        exit(1);
    }
}

void unlock() {
    if (semop(semid, &v, 1) == -1) {
        perror("semop unlock");
        exit(1);
    }
}

void initializeStore() {
    shmid = shmget(SHM_KEY, sizeof(KeyValue) * MAX_KEYS, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    store = (KeyValue *) shmat(shmid, NULL, 0);
    if (store == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl failed");
        exit(1);
    }
}

int put(char* key, char* value) {
    lock();
    for (int i = 0; i < MAX_KEYS; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(store[i].value, value, sizeof(store[i].value) - 1);
            unlock();
            return 1; // Wert überschrieben
        }
    }
    for (int i = 0; i < MAX_KEYS; i++) {
        if (store[i].key[0] == '\0') {
            strncpy(store[i].key, key, sizeof(store[i].key) - 1);
            strncpy(store[i].value, value, sizeof(store[i].value) - 1);
            unlock();
            return 0; // Neuer Wert hinzugefügt
        }
    }
    unlock();
    return -1; // Kein Platz mehr
}

int get(char* key, char* res) {
    lock();
    for (int i = 0; i < MAX_KEYS; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(res, store[i].value, 255);
            unlock();
            return 0; // Wert gefunden
        }
    }
    unlock();
    return -1; // Schlüssel nicht gefunden
}

int del(char* key) {
    lock();
    for (int i = 0; i < MAX_KEYS; i++) {
        if (strcmp(store[i].key, key) == 0) {
            store[i].key[0] = '\0'; // Key löschen
            store[i].value[0] = '\0'; // Wert löschen
            unlock();
            return 0; // Wert gelöscht
        }
    }
    unlock();
    return -1; // Schlüssel nicht gefunden
}
