#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include "RingBuff.h"

pthread_mutex_t RingBuff::m_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  RingBuff::m_cond_r = PTHREAD_COND_INITIALIZER;
pthread_cond_t  RingBuff::m_cond_w = PTHREAD_COND_INITIALIZER;

typedef void *(*THREAD_ENTRY)(void *);

RingBuff  *ring_buff;

void *consumer(void *arg)
{
    char  name[32] = {0};
    unsigned char  data[64] = {0};

    pthread_getname_np(pthread_self(), name, sizeof(name));
    
    while (1)
    {
        memset(data, 0, sizeof(data));
        //int r_size = ring_buff->Read(data, ring_buff->Length());  //read all data
        int r_size = ring_buff->Read(data, 4);  //read part data
        cout << name << " read  [" << data << "] size[" << r_size << "]" << endl;
    }
}

void *producer(void *arg)
{
    char  name[32] = {0};
    char  data[32] = {0};

    strcpy(data, (char *)arg);
    free(arg);

    pthread_getname_np(pthread_self(), name, sizeof(name));
    
    while (1)
    {
        int w_size = ring_buff->Write((unsigned char *)data, strlen(data));
        cout << name << " write [" << data << "] size[" << w_size << "]" << endl;
    }
}

int thread_create(pthread_t *task_id, const char *name, THREAD_ENTRY entry, void *argument) 
{
    pthread_attr_t  task_attr;

    if (task_id == NULL || name == NULL || entry == NULL)
    {
        return 0;
    }

    do
    {
        if (0 != pthread_attr_init(&task_attr))
        {
            break;
        }

        if (0 != pthread_attr_setdetachstate(&task_attr, PTHREAD_CREATE_DETACHED))
        {
            break;
        }
        
        if (0 != pthread_create(task_id, &task_attr, entry, argument))
        {
            break;
        }

        if (0 != pthread_setname_np(*task_id, name))
        {
            break;
        }

        pthread_attr_destroy(&task_attr); 
        return 0;
    } while (0);

    return -1;
}

int main(int argc, char *argv[])
{
    int  queue_capacity;
    int  consumer_count;
    int  producer_count;
    pthread_t  task_id;

    if (argc != 4)
    {
        cout << "CMD <queue_capacity> <consumer_count> <producer_count>" << endl;
        return -1;
    }

    queue_capacity = atoi(argv[1]);
    consumer_count = atoi(argv[2]);
    producer_count = atoi(argv[3]);

    ring_buff = new RingBuff(queue_capacity);

    for (int i = 0; i < producer_count; i++)
    {
        char  thread_name[32] = {0};
        char  *data = (char *)malloc(32 * sizeof(char));
        snprintf(data, 32, "this is data %d", i);
        snprintf(thread_name, sizeof(thread_name), "producer_%d", i);
        thread_create(&task_id, thread_name, producer, data);
    }

    for (int i = 0; i < consumer_count; i++)
    {
        char  thread_name[32] = {0};
        snprintf(thread_name, sizeof(thread_name), "consumer_%d", i);
        thread_create(&task_id, thread_name, consumer, NULL);
    }

    while (1)
    {
        sleep(10);
    }
    
    return 0;
}

