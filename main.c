#include "main.h"
#include "keyValStore.h"
#include "sub.h"

int main() {
    // Initialisierung des Key-Value-Stores
    initializeStore();

    // Beispielhafte Verwendung der Store-Funktionen
    put("key1", "value1");
    char result[256]; // Annahme, dass Werte nicht länger als 255 Zeichen sind
    if (get("key1", result) >= 0) {
        printf("Value retrieved: %s\n", result);
    } else {
        printf("Key not found\n");
    }

    del("key1");
    if (get("key1", result) < 0) {
        printf("Key correctly deleted\n");
    }

    // Bereinigung und Schließen des Programms
    cleanupStore();
    return 0;
    // Cem
}