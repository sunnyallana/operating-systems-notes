#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#define NUM_READERS 5
#define NUM_WRITERS 2
#define STRING_LENGTH 60

pthread_t reader_threads[NUM_READERS], writer_threads[NUM_WRITERS];
sem_t mutex, rw_mutex;
int readers_count = 0;
FILE *file;

char generateRandomChar() {
    return (char)('a' + rand() % 26);
}

void *reader(void *arg) {
    while (1) {
        sem_wait(&mutex);
        readers_count++;
        if (readers_count == 1) {
            sem_wait(&rw_mutex);
        }
        sem_post(&mutex);

        fseek(file, 0, SEEK_SET);
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            fprintf(stdout, "Reader %ld: %s", (long)arg, buffer);
        }

        sem_wait(&mutex);
        readers_count--;
        if (readers_count == 0) {
            sem_post(&rw_mutex);
        }
        sem_post(&mutex);

        usleep(1000);
    }
}

void *writer(void *arg) {
    while (1) {
        sem_wait(&rw_mutex);

        srand(time(NULL));
        char randomString[STRING_LENGTH + 1];
        for (int i = 0; i < STRING_LENGTH; i++) {
            randomString[i] = generateRandomChar();
        }
        randomString[STRING_LENGTH] = '\0';

        fseek(file, 0, SEEK_END);
        fprintf(file, "%s\n", randomString);
        fprintf(stdout, "Writer %ld: %s\n", (long)arg, randomString);
        fflush(file);

        sem_post(&rw_mutex);

        usleep(1000);
    }
}

int main() {
    file = fopen("shared_file.txt", "a+");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    sem_init(&mutex, 0, 1);
    sem_init(&rw_mutex, 0, 1);

    int i;
    for (i = 0; i < NUM_WRITERS; i++) pthread_create(&writer_threads[i], NULL, writer, (void *)(long)i);
    for (i = 0; i < NUM_READERS; i++) pthread_create(&reader_threads[i], NULL, reader, (void *)(long)i);

    for (i = 0; i < NUM_READERS; i++) pthread_join(reader_threads[i], NULL);
    for (i = 0; i < NUM_WRITERS; i++) pthread_join(writer_threads[i], NULL);

    sem_destroy(&mutex);
    sem_destroy(&rw_mutex);
    fclose(file);

    return 0;
}
