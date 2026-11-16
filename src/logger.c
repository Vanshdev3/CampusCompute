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
        time_t now = time(NULL); struct tm tm_value; char stamp[32] = "time-unavailable";
        if (localtime_r(&now, &tm_value)) strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm_value);
        fprintf(l->file, "%s | event=%d | %s\n", stamp, e.id, e.message); fflush(l->file);
        printf("Consumer wrote event %d: %s\n", e.id, e.message);
    }
    return NULL;
}

int run_logger(int event_count, const char *path) {
    Logger l = {.total = event_count}; pthread_t p, c;
    int mutex_ready = 0, empty_ready = 0, full_ready = 0;
    if (event_count < 1 || !path || path[0] == '\0') { fprintf(stderr, "logger requires a positive event count and non-empty log path.\n"); return 1; }
    l.file = fopen(path, "w"); if (!l.file) { perror("fopen log"); return 1; }
    int rc = pthread_mutex_init(&l.mutex, NULL);
    if (rc != 0) { fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc)); goto fail; }
    mutex_ready = 1;
    rc = pthread_cond_init(&l.not_empty, NULL);
    if (rc != 0) { fprintf(stderr, "pthread_cond_init not_empty: %s\n", strerror(rc)); goto fail; }
    empty_ready = 1;
    rc = pthread_cond_init(&l.not_full, NULL);
    if (rc != 0) { fprintf(stderr, "pthread_cond_init not_full: %s\n", strerror(rc)); goto fail; }
    full_ready = 1;
    rc = pthread_create(&c, NULL, consumer, &l);
    if (rc != 0) { fprintf(stderr, "pthread_create consumer: %s\n", strerror(rc)); goto fail; }
    rc = pthread_create(&p, NULL, producer, &l);
    if (rc != 0) {
        fprintf(stderr, "pthread_create producer: %s\n", strerror(rc));
        pthread_mutex_lock(&l.mutex); l.closed = 1; pthread_cond_broadcast(&l.not_empty); pthread_mutex_unlock(&l.mutex);
        pthread_join(c, NULL); goto fail;
    }
    int producer_join = pthread_join(p, NULL);
    int consumer_join = pthread_join(c, NULL);
    if (producer_join != 0 || consumer_join != 0) {
        if (producer_join != 0) fprintf(stderr, "pthread_join producer: %s\n", strerror(producer_join));
        if (consumer_join != 0) fprintf(stderr, "pthread_join consumer: %s\n", strerror(consumer_join));
        goto fail;
    }
    puts("Producer thread joined; consumer thread joined.");
    printf("Logger stopped cleanly; %d events written to %s\n", event_count, path);
    pthread_cond_destroy(&l.not_full); pthread_cond_destroy(&l.not_empty); pthread_mutex_destroy(&l.mutex); fclose(l.file); return 0;
fail:
    if (full_ready) pthread_cond_destroy(&l.not_full);
    if (empty_ready) pthread_cond_destroy(&l.not_empty);
    if (mutex_ready) pthread_mutex_destroy(&l.mutex);
    fclose(l.file); return 1;
}
