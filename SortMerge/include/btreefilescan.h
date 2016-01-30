/* -*- C++ -*- */
/*
 * btreefilescan.h - definition of class BTreeFileScan
 *
 */
 
#ifndef _BTREE_FILESCAN_H
#define _BTREE_FILESCAN_H

#include "btfile.h"

/*
 * BTreeFileScan implements a search/iterate interface to B+ tree 
 * index files (class BTreeFile).  It derives from abstract base
 * class IndexFileScan.  It uses class BTreeFile's error values
 * (for the erroneous error protocol).
 */

class BTreeFileScan : public IndexFileScan {

  public:
    friend class BTreeFile;

    // get the next record. NOTE: returns DONE instead of NOMORERECS when
    // finished.  (Our page code returns NOMORERECS in accordance with
    // class RecordPage; however, main.cc wants to see DONE.  It should
    // probably be looking for NOMORERECS, but a workaround is easy enough.)
    Status get_next(RID & rid, void* keyptr);

    // delete the record currently scanned
    Status delete_current();

    int keysize(); // size of the key

   ~BTreeFileScan();

 private:
    BTreeFile *treep;           // B+ tree we're scanning 

    BTLeafPage *leafp;          // leaf page containing current record
    RID curRid;                 // position in current leaf; note: this is 
                                // the RID of the key/RID pair within the
                                // leaf page.

    bool didfirst;              // false only before get_next is called
    bool deletedcurrent;        // true after delete_current is called (read
                                // by get_next, written by delete_current).
    
    const void *endkey;         // if 0, then go all the way right
                                // else, stop when current record > this value.
                                // (that is, implement an inclusive range 
                                // scan -- the only way to do a search for 
                                // a single value).
};

#endif  // _BTREE_FILESCAN_H
