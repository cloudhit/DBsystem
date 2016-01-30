/*
 * implementation of class Scan for HeapFile project.
 * $Id: scan.C,v 1.1 1997/01/02 12:46:42 flisakow Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "heapfile.h"
#include "scan.h"
#include "hfpage.h"
#include "buf.h"
#include "db.h"

// *******************************************
Scan::Scan (HeapFile *hf, Status& status)
{
    status = init(hf);
}

// *******************************************
Scan::~Scan()
{
    reset();
}

// *******************************************
Status Scan::init(HeapFile *hf)
{
    _hf = hf;

    return firstDataPage();
}

// *******************************************
Status Scan::reset()
{
    Status st = OK;

    if (datapage != NULL) {
        st = MINIBASE_BM->unpinPage(datapageId);
        if (st != OK)
            return MINIBASE_CHAIN_ERROR( HEAPFILE, st);
    }
    datapageId = 0;
    datapage = NULL;

    nxtUserStatus = OK;

    return st;
}

// *******************************************
Status Scan::firstDataPage()
{
    Status    st;

      // copy data about first page
    datapageId = _hf->_firstPageId;  

    nxtUserStatus = OK;
    datapage = NULL;

      // Actually get the page
    st = nextDataPage();

    if ((st != OK) && (st != DONE))
        return MINIBASE_CHAIN_ERROR( HEAPFILE, st);

    return OK;
}

// *******************************************
// Retrieve the next data page
Status Scan::nextDataPage()
{
    Status   st;
    PageId   nextDataPageId;
  
    if (datapage == NULL) {
        if (datapageId == INVALID_PAGE) {
            // heapfile is empty to begin with
            return DONE;
        } else {
            // pin first data page
            st = MINIBASE_BM->pinPage(datapageId, (Page *&) datapage);
            if (st != OK)
                return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
           
            // find the first record
            st = datapage->firstRecord(userrid);

            if (st != DONE) {
                if (st != OK)
                    return MINIBASE_CHAIN_ERROR( HEAPFILE, st );
      
                return OK;
            }
        }
    }
 
    nextDataPageId = datapage->getNextPage();
 
    // unpin the current datapage
    st = MINIBASE_BM->unpinPage(datapageId);
    datapage = NULL;
    if (st != OK)
        return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
   
    datapageId = nextDataPageId;

    if (datapageId == INVALID_PAGE)
        return DONE;
 
    st = MINIBASE_BM->pinPage(datapageId, (Page *&) datapage);
    if (st != OK)
        return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
   
    nxtUserStatus = datapage->firstRecord(userrid);

    if (nxtUserStatus != OK)
        return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
    return OK;
}

// *******************************************
// Retrieve the next record in a sequential scan.
// Also returns the RID of the retrieved record.
Status Scan::getNext(RID& rid, char *recPtr, int& recLen)
{
    Status st;

    if (nxtUserStatus != OK)
        nextDataPage();

    if (datapage == NULL)
        return DONE;

    rid = userrid;     
    st  = datapage->getRecord(rid, recPtr, recLen);

    if (st != OK)
        return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

    nxtUserStatus = datapage->nextRecord(rid, userrid);

    return st;
}

// *******************************************
// Position the scan cursor to the record with the given rid.
// Returns OK if successful, non-OK otherwise.
Status Scan::position(RID rid)
{
    Status st;
    RID    nxtrid;

    st = peekNext(nxtrid);

    if (nxtrid == rid)
        return OK;

    // This is kinda lame, but otherwise it will take all day.
    PageId pgid = rid.pageNo;
 
    if (datapageId != pgid) {

        // reset everything and start over from the beginning
        reset();
        st = firstDataPage();

        if (st != OK)
            return MINIBASE_CHAIN_ERROR(HEAPFILE, st);

        while (datapageId != pgid) {
            st = nextDataPage();
            if (st != OK)
                return MINIBASE_CHAIN_ERROR(HEAPFILE, st);
        }
    }


    // Now we are on the correct page.

    st = datapage->firstRecord(userrid);
    if (st != OK)
        return MINIBASE_CHAIN_ERROR(HEAPFILE, st);

    st = peekNext(nxtrid);

    while ((st == OK) && (nxtrid != rid))
        st = mvNext(nxtrid);

    return st;
}

// *******************************************
// Move to the next record in a sequential scan.
// Also returns the RID of the (new) current record.
Status Scan::mvNext(RID& rid)
{
    Status st = OK;

    if (datapage == NULL)
        return DONE;

    nxtUserStatus = datapage->nextRecord(userrid, rid);

    if (nxtUserStatus == OK) {
        userrid = rid;               // save rid
        return st;
    } else {
        st = nextDataPage();
        if (st == OK)
            rid = userrid;
    }
  
    return st;
}

// *******************************************
