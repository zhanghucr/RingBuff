#include "RingBuff.h"

RingBuff::RingBuff(unsigned int capacity) : m_capacity(capacity), m_size(0), m_read(0), m_write(0)
{    
    m_buff = new (std::nothrow) unsigned char[capacity];
    if (m_buff != NULL)
    {
        memset(m_buff, 0, capacity);
    }
}

RingBuff::~RingBuff()
{
    delete[] m_buff;
}

unsigned int RingBuff::Write(const unsigned char *data, unsigned int size)
{
    if (data == NULL || size == 0 || size > m_capacity)
    {
        return 0;
    }

    /* wait condition variable */
    pthread_mutex_lock(&m_mutex);
    if (m_capacity - m_size < size)
    {
        pthread_cond_wait(&m_cond_w, &m_mutex);
    }

    if (m_write >= m_read)
    {
        /*
        |<----------- capacity ----------->|
        +--------+---------------+---------+
        |        |***************|         |
        +--------+---------------+---------+
                 ^               ^
               read            write
        */
        if (m_capacity - m_write >= size)
        {
            memcpy(&m_buff[m_write], data, size);
        }
        else
        {
            memcpy(&m_buff[m_write], data, m_capacity - m_write);
            memcpy(m_buff, &data[m_capacity - m_write], size - (m_capacity - m_write));
        }
    }
    else
    {
        /*
        |<----------- capacity ----------->|
        +--------+---------------+---------+
        |********|               |*********|
        +--------+---------------+---------+
                 ^               ^
               write           read
        */
        memcpy(&m_buff[m_write], data, size);
    }

    m_write += size;
    m_write %= m_capacity;
    m_size += size;

    pthread_cond_signal(&m_cond_r);
    pthread_mutex_unlock(&m_mutex);
    return size;
}

unsigned int RingBuff::Read(unsigned char *data, unsigned int size)
{
    unsigned int copy_size;
    
    if (data == NULL || size == 0)
    {
        return 0;
    }

    /* wait condition variable */
    pthread_mutex_lock(&m_mutex);
    if (isEmpty())
    {
        pthread_cond_wait(&m_cond_r, &m_mutex);
    }

    if (size <= m_size)
    {
        copy_size = size;
    }
    else
    {
        copy_size = m_size;
    }

    if (m_write > m_read)
    {
        /*
        |<----------- capacity ----------->|
        +--------+---------------+---------+
        |        |***************|         |
        +--------+---------------+---------+
                 ^               ^
               read            write
        */

        memcpy(data, &m_buff[m_read], copy_size);
    }
    else
    {
        /*
        |<----------- capacity ----------->|
        +--------+---------------+---------+
        |********|               |*********|
        +--------+---------------+---------+
                 ^               ^
               write           read
        */

        if (m_capacity - m_read >= copy_size)
        {
            memcpy(data, &m_buff[m_read], copy_size);
        }
        else
        {
            memcpy(data, &m_buff[m_read], m_capacity - m_read);
            memcpy(&data[m_capacity - m_read], m_buff, copy_size - (m_capacity - m_read));
        }
    }

    m_read += copy_size;
    m_read %= m_capacity;
    m_size -= copy_size;

    pthread_cond_signal(&m_cond_w);
    pthread_mutex_unlock(&m_mutex);
    return copy_size;
}

void RingBuff::Flush()
{
    m_read = 0;
    m_write = 0;
    m_size = 0;
}

unsigned int RingBuff::Length()
{
    return m_size;
}

unsigned int RingBuff::Remain()
{
    return m_capacity - m_size;
}

bool RingBuff::isEmpty()
{ 
    return m_size == 0;
}

bool RingBuff::isFull()
{
    return m_size == m_capacity;
}

