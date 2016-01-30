/*
 * btfile.h
 *
 * sample header file
 *
 */
 
#ifndef _BTREE_H
#define _BTREE_H

#include "btindex_page.h"
#include "btleaf_page.h"
#include "index.h"
#include "btreefilescan.h"
#include "bt.h"

// Define your error code for B+ tree here
enum btErrCodes  {TYPEERROR,LOSTTOFIND, NOMORERECORD};
class BTreeHeaderPage{
public:
    PageId root;
    AttrType key_type;
    int keysize;
};
class BTreeFile: public IndexFile
{
private:
  public:
    BTreeHeaderPage* root_page;
    PageId root;
    char* filename;
    BTreeFile(Status& status, const char *filename);
    // an index with given filename should already exist,
    // this opens it.
    
    BTreeFile(Status& status, const char *filename, const AttrType keytype,  \
	      const int keysize);
    // if index exists, open it; else create it.
    
    ~BTreeFile();
    // closes index
    
    char* getmidkey(SortedPage* leaf, const void* key);
    void addRoot(SortedPage* leaf, char* result_key);
    bool DFS(PageId pageid);
    Status destroyFile();
    // destroy entire index file, including the header page and the file entry
    
    Status insert(const void *key, const RID rid);
    // insert <key,rid> into appropriate leaf page
    
    Status Delete(const void *key, const RID rid);
    // delete leaf entry <key,rid> from the appropriate leaf
    // you need not implement merging of pages when occupancy
    // falls below the minimum threshold (unless you want extra credit!)
    
    IndexFileScan *new_scan(const void *lo_key = NULL, const void *hi_key = NULL);
    // create a scan with given keys
    // Cases:
    //      (1) lo_key = NULL, hi_key = NULL
    //              scan the whole index
    //      (2) lo_key = NULL, hi_key!= NULL
    //              range scan from min to the hi_key
    //      (3) lo_key!= NULL, hi_key = NULL
    //              range scan from the lo_key to max
    //      (4) lo_key!= NULL, hi_key!= NULL, lo_key = hi_key
    //              exact match ( might not unique)
    //      (5) lo_key!= NULL, hi_key!= NULL, lo_key < hi_key
    //              range scan from lo_key to hi_key

    int keysize();
    PageId getLeafPageNo(void* key, bool high= TRUE);
  private:

};

#endif
