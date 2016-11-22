#ifndef _QUERY_OBJECT_H
#define _QUERY_OBJECT_H

struct query_object
{
    virtual void run() = 0;
    virtual ~query_object() = default;
};

#endif
