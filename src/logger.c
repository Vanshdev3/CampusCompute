#include "campuscompute.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QUEUE_CAPACITY 4
typedef struct { int id; char message[96]; } Event;
typedef struct {
    Event queue[QUEUE_CAPACITY]; size_t head, tail, count; int closed, total;
    pthread_mutex_t mutex; pthread_cond_t not_empty, not_full; FILE *file;
} Logger;

static void *producer(void *arg) {
    Logger *l = arg;
    for (int i = 1; i <= l->total; ++i) {
        Event e = {.id = i}; snprintf(e.message, sizeof(e.message), "job J%d state changed to %s", (i % 4) + 1, i % 2 ? "RUNNING" : "COMPLETED");
        pthread_mutex_lock(&l->mutex);
        while (l->count == QUEUE_CAPACITY) pthread_cond_wait(&l->not_full, &l->mutex);
        l->queue[l->tail] = e; l->tail = (l->tail + 1) % QUEUE_CAPACITY; ++l->count;
        printf("Producer queued event %d (queue=%zu/%d)\n", e.id, l->count, QUEUE_CAPACITY);
        pthread_cond_signal(&l->not_empty); pthread_mutex_unlock(&l->mutex);
    }
    pthread_mutex_lock(&l->mutex); l->closed = 1; pthread_cond_broadcast(&l->not_empty); pthread_mutex_unlock(&l->mutex);
    return NULL;
}

static void *consumer(void *arg) {
    Logger *l = arg;
    for (;;) {
        pthread_mutex_lock(&l->mutex);
        while (!l->count && !l->closed) pthread_cond_wait(&l->not_empty, &l->mutex);
        if (!l->count && l->closed) { pthread_mutex_unlock(&l->mutex); break; }
        Event e = l->queue[l->head]; l->head = (l->head + 1) % QUEUE_CAPACITY; --l->count;
        pthread_cond_signal(&l->not_full); pthread_mutex_unlock(&l->mutex);
        time_t now = time(NULL); struct tm *tm = localtime(&now); char stamp[32]; strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", tm);
        fprintf(l->file, "%s | event=%d | %s\n", stamp, e.id, e.message); fflush(l->file);
        printf("Consumer wrote event %d: %s\n", e.id, e.message);
    }
    return NULL;
}

int run_logger(int event_count, const char *path) {
    Logger l = {.total = event_count}; pthread_t p, c; int initialized = 0;
    l.file = fopen(path, "w"); if (!l.file) { perror("fopen log"); return 1; }
    if (pthread_mutex_init(&l.mutex, NULL) || pthread_cond_init(&l.not_empty, NULL) || pthread_cond_init(&l.not_full, NULL)) { fprintf(stderr, "logger synchronization initialization failed\n"); fclose(l.file); return 1; }
    initialized = 1;
    if (pthread_create(&c, NULL, consumer, &l)) { perror("pthread_create consumer"); goto fail; }
    if (pthread_create(&p, NULL, producer, &l)) { perror("pthread_create producer"); pthread_mutex_lock(&l.mutex); l.closed=1; pthread_cond_broadcast(&l.not_empty); pthread_mutex_unlock(&l.mutex); pthread_join(c,NULL); goto fail; }
    pthread_join(p, NULL); pthread_join(c, NULL); printf("Logger stopped cleanly; %d events written to %s\n", event_count, path);
    pthread_cond_destroy(&l.not_full); pthread_cond_destroy(&l.not_empty); pthread_mutex_destroy(&l.mutex); fclose(l.file); return 0;
fail:
    if (initialized) { pthread_cond_destroy(&l.not_full); pthread_cond_destroy(&l.not_empty); pthread_mutex_destroy(&l.mutex); }
    fclose(l.file); return 1;
}
