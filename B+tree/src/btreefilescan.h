/*
 * btreefilescan.h
 *
 * sample header file
 *
 */
 
#ifndef _BTREEFILESCAN_H
#define _BTREEFILESCAN_H

#include "btfile.h"

// errors from this class should be defined in btfile.h
class metadata{
 public:
 RID rid;
 char* key;
 metadata* next;
 metadata(RID rid, char* key){this->rid = rid; this->key = key;next = 0;}
};
class BTreeFileScan : public IndexFileScan {
public:
    friend class BTreeFile;

    // get the next record
    Status get_next(RID & rid, void* keyptr);

    // delete the record currently scanned
    Status delete_current();

    int keysize(); // size of the key

    // destructor
    ~BTreeFileScan();
    void setkeysize(int keysize);
    metadata* head = 0;
    metadata* tail = 0;
    metadata* cur = 0;
    const char* filename; 
private:
    int key_size;
};

#endif
