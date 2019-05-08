#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__

#include <iostream>
#include <cstring>
#include <pthread.h>

using namespace std;

class RingBuff
{
public:
    RingBuff(unsigned int size);
    virtual ~RingBuff();

    bool isFull();
    bool isEmpty();
    unsigned int Length();
    unsigned int Remain();
    unsigned int Write(const unsigned char *data, unsigned int size);
    unsigned int Read(unsigned char *data, unsigned int size);
    void Flush();

public:
    static pthread_mutex_t  m_mutex;
    static pthread_cond_t   m_cond_r;  /* read condition */
    static pthread_cond_t   m_cond_w;  /* write condition */
private:
    unsigned char *m_buff;
    unsigned int   m_capacity;
    unsigned int   m_size;
    unsigned int   m_read;
    unsigned int   m_write;
};

#endif  /* __RING_BUFF_H__ */

