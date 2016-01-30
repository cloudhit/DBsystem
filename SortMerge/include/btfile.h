/* -*- C++ -*- */
/*
 * btfile.h - definition of class BTreeFile : public IndexFile
 *
 */

#ifndef _BTFILE_H
#define _BTFILE_H

/*
 * This is the main definition of class BTreeFile, which derives from 
 * abstract base class IndexFile.
 *
 * class BTreeFileScan is defined in btreefilescan.h; this class provides
 * a search/iterate interface to a BTreeFile.
 */


#define BT_TRACE
  /* When #defined, BT_TRACE causes a structured trace to be written to the
     ostream pointed to by the class variable BTreeFile::Trace.  This output is
     used to drive a visualization tool that shows the inner workings of the
     b-tree during its operations.  You must set BTreeFile::Trace to the
     ostream of your choice before creating your BTreeFile. */
 
#include "btindex_page.h"
#include "btleaf_page.h"
#include "index.h"
//#include "btreefilescan.h"
#include "bt.h"

#define NAIVE_DELETE 0
#define FULL_DELETE  1

class BTreeFile: public IndexFile {

  friend class BTreeFileScan;
  friend class BTreeTest;

  /*
   * Structure of a B+ tree index header page.  There is quite a bit 
   * of wasted space here... 
   */

  private:
    struct slot_t {
        short   offset;  
        short   length;       // equals INVALID_PAGE if slot is not in use
    };

    const static int DPFIXED =       sizeof(slot_t)
                        + 4 * sizeof(short)
                        + 3 * sizeof(PageId);

    struct BTreeHeaderPage {
        unsigned long magic0; // magic number for sanity checking
    
        PageId root;         // page containing root of tree
        AttrType key_type;   // type of keys in tree
        int keysize;         // max key length (specified at index creation)
        int delete_fashion;  // naive delete algorithm or full delete algorithm

        /*
         * Note that we need not store the "file name" associated with this
         * index because the name is how the index is found in the first 
         * place (the name is the index of the index, heh).  We always
         * get the name as in-core data via BTreeFile::BTreeFile and copy 
         * it off there.
         */
    };

  public:

    enum ErrorTypes { 

      // our interface for the erroneous error protocol.

      _OK = 0,                // 0 is always OK
      CANT_FIND_HEADER,       // tried to open index but db said no header
      CANT_PIN_HEADER,        // buffer manager failed to pin header page
      CANT_ALLOC_HEADER,      // failed to allocate block for header page
      CANT_ADD_FILE_ENTRY,    // couldn't register new index file w/ db
      CANT_UNPIN_HEADER,      // can't unpin header page
      CANT_PIN_PAGE,          // can't pin some index/leaf page
      CANT_UNPIN_PAGE,        // can't unpin some index/leaf page
      INVALID_SCAN,           // attempt to use bad Scan object 
      DELETE_CURRENT_FAILED,  // failed to delete current rid in scan
      CANT_DELETE_FILE_ENTRY, // db failed to delete file entry
      CANT_FREE_PAGE,         // buffer manager failed to free a page
      CANT_DELETE_SUBTREE,    // _destroyFile failed on a subtree
      KEY_TOO_LONG,           // the key given in BTreeFile::insert is too long
      INSERT_FAILED,          // BTreeFile::insert not successful
      COULD_NOT_CREATE_ROOT,  // BtreeFile::insert could not create new root
      DELETE_DATAENTRY_FAILED,// could not delete a data entry 
      DATA_ENTRY_NOT_FOUND,   // could not find data entry to delete
      CANT_GET_PAGE_NO,       // get_page_no on BTIndexPage failed
      CANT_ALLOCATE_NEW_PAGE, // bm::newPage failed
      CANT_SPLIT_LEAF_PAGE,   // could not split leaf page
      CANT_SPLIT_INDEX_PAGE,  // could not split index page
  
      NR_ERRORS               // and this is the number of them
    }; 
  
    static const char *Errors[NR_ERRORS];

    // an index with given filename should already exist; this opens it.
    BTreeFile(Status& status, const char *filename);
  
    // if index exists, open it; else create it.
    BTreeFile(Status& status, const char *filename, const AttrType keytype,
              const int keysize, int delete_fashion = FULL_DELETE);
    
    // closes index
   ~BTreeFile();
    
    // destroy entire index file
    Status destroyFile();
    
    // insert recid with the key key
    Status insert(const void *key, const RID rid);
    
    // delete leaf entry recid given its key
    // (`rid' is IN the data entry; it is not the id of the data entry)
    Status Delete(const void *key, const RID rid);
    
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
    IndexFileScan *new_scan(const void *lo_key = NULL,
                            const void *hi_key = NULL);

    int keysize();
    

    void PrintHeader();            // print the header info
    void PrintRoot();              // print the root page
    void PrintLeafPages();         // print all the leaf pages
    void PrintPage(PageId id);     // print page with id

#if defined(BT_TRACE)
    static ostream *Trace;   // Set this to your stream to get a trace.
#endif

  private:

    BTreeHeaderPage *headerPage;   // (header page)
    PageId           headerPageId; // page number of header page
    char            *dbname;       // copied from arg of the ctor.

    
    // Private member functions:

    // Change the root of the tree to the specified page. 
    Status updateHeader (PageId newRoot);  
  
    // Recursively insert a new data entry <key,rid> , 
    // returning pushed-up/copied-up index page entry (*goingUp)
    // when we split (*goingUp is NULL when split stops). 
    // Inserts onto page currentPageId.
    Status _insert (const void    *key,
                    const RID      rid,
                    KeyDataEntry **goingUp,
                    int           *goingUpSize,
                    PageId         currentPageId);

    Status FullDelete(const void *key, const RID rid);

    Status NaiveDelete(const void *key, const RID rid);

    Status _Delete (const void    *key,
                     const RID     rid,
                     void        *&oldChildEntry,
                     PageId        currentPageId,
                     PageId        parentPageId);


    // findRunStart:  return the pinned page containing the left-most 
    // occurrence of key value `lo_key'.  Also returns the RID (in the data 
    // entry) corresponding to the first occurrence of key lo_key.
    //
    // This function is somewhat complex due to the fact that we handle
    // duplicates (EC 1) and the fact that we don't do the standard delete 
    // algorithm and so some leaf pages may become empty.
    Status findRunStart (const void *lo_key, BTLeafPage **ppage, RID *prid);
  
    // _destroyFile: recursively destroy the tree rooted at a specified page.
    Status _destroyFile (PageId pageno);

#if defined(BT_TRACE)
    void trace_children(PageId id);  // Print trace of a page's children.
#endif

};

#endif // _BTFILE_H
