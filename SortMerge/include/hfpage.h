// **********************************************
//     Heap File Page Class
//     $Id: hfpage.h,v 1.2 1997/01/04 09:48:34 flisakow Exp $
// **********************************************

#ifndef _HFPAGE_H
#define _HFPAGE_H

#include "minirel.h"
#include "page.h"

// Class definition for a microbase data page.   
// Notice, this class does not keep the records aligned,
// relying instead on upper levels to take
// care of non-aligned attributes.

// This version assumes all records being stored on
// a page will be the same size.  It considers the data space
// to be an array of records, with a bitmap indicating which
// records are valid.

// These constants limit the size of the bitmap, and thus
// the number of records a page may contain.
const int BITMAP_SIZE       =  16;    // in bytes
const int MAX_RECS_PER_PAGE = 128;    // BITMAP_SIZE * 8

class HFPage {

  protected:

    static const int DPFIXED =       BITMAP_SIZE
                           + 4 * sizeof(short)
                           + 3 * sizeof(PageId);

      // Warning:
      // These items must all pack tight, (no padding) for
      // the current implementation to work properly.
      // Be careful when modifying this class.

    short     recSze;      // The size of the records being stored.
    short     recCnt;      // number of records in use.
    short     maxRecs;     // the max number of records (at this size).

    short     type;        // an arbitrary value used by subclasses as needed

    PageId    prevPage;    // backward pointer to data page.
    PageId    nextPage;    // forward pointer to data page.
    PageId    curPage;     // page number of this page.

    char      bitmap[BITMAP_SIZE];    // The bitmap of used records.

    char      data[MAX_SPACE - DPFIXED]; 

  public:
    void init(PageId pageNo);   // initialize a new page.
    void dumpPage();            // dump contents of a page.

    PageId getNextPage();       // returns value of nextPage.
    PageId getPrevPage();       // returns value of prevPage.

    void setNextPage(PageId pageNo);    // sets value of nextPage to pageNo
    void setPrevPage(PageId pageNo);    // sets value of prevPage to pageNo

    void  set_type(short t) { type = t; }
    short get_type() { return type; }

    PageId page_no() { return curPage;} // returns the page number

    // inserts a new record pointed to by recPtr with length recLen onto
    // the page, returns RID of record 
    Status insertRecord(char *recPtr, int recLen, RID& rid);

    // delete the record with the specified rid
    Status deleteRecord(const RID& rid);

      // returns RID of first record on page
      // returns DONE if page contains no records.  Otherwise, returns OK
    Status firstRecord(RID& firstRid);

      // returns RID of next record on the page 
      // returns DONE if no more records exist on the page
    Status nextRecord (RID curRid, RID& nextRid);

      // copies out record with RID rid into recPtr
    Status getRecord(RID rid, char *recPtr, int& recLen);

      // returns a pointer to the record with RID rid
    Status returnRecord(RID rid, char*& recPtr, int& recLen);

      // returns the amount of available space on the page
    int available_space(void);

      // Returns true if the HFPage is has no records in it, false otherwise.
    bool empty(void);

      // The number of records on this page.
    int  num_recs() { return recCnt; }

  protected:
    // Compacts the slot directory on an HFPage.
    // WARNING -- this will probably lead to a change in the RIDs of
    //    records on the page.  You CAN'T DO THIS on most kinds of pages.
    void compact_slot_dir();
};

#endif // _HFPAGE_H
