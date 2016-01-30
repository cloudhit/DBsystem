/* -*- C++ -*- */
/*
 * btindex_page.h - definition of class BTIndexPage for Mini{base|rel} project.
 *
 */

#ifndef BTINDEX_PAGE_H
#define BTINDEX_PAGE_H


#include "minirel.h"
#include "page.h"
#include "sorted_page.h"
#include "bt.h"


/*
 * A BTIndexPage is an index page on a B+ tree.  It holds abstract 
 * {key, PageId} pairs; it doesn't know anything about the keys 
 * (their lengths or their types), instead relying on the abstract
 * interface consisting of keyCompare(), make_entry(), get_key_data(),
 * and get_key[_data]_len() from bt.h and key.cc.  
 * See those files for our {key,data} pairing interface and implementation.
 */

class BTIndexPage : public SortedPage {
 private:
   // No private variables should be declared.

 public:

/*
 * Error handling infrastructure, added by us:
 */

  enum ErrorTypes { 
    _OK = 0,
    INDEXINSERTRECFAILED,
    INDEXNR_ERRORS              // and this is the number of them
  }; 

   static const char *Errors[INDEXNR_ERRORS];


// In addition to initializing the  slot directory and internal structure
// of the HFPage, this function sets up the type of the record page.

   void init(PageId pageNo) {
       HFPage::init(pageNo);
       set_type(INDEX);
   }

// ------------------- insertKey ------------------------
// Inserts a <key, page pointer> value into the index node.
// This is accomplished by a call to SortedPage::insertRecord()
// The function also sets up the recPtr field for the call to
// SortedPage::insertRecord()
   
   Status insertKey(const void *key, AttrType key_type,
                    PageId pageNo, RID& rid);

// ------------------ OPTIONAL: deletekey ------------------
// This is optional, and is only needed if you want to do full deletion.
   Status deleteKey(const void *key, AttrType key_type, RID& curRid);

// ------------------ get_page_no -----------------------
// This function encapsulates the search routine to search a
// BTIndexPage. It uses the standard search routine as
// described on page 77 of the textbook. It returns the page_no
// of the child to be searched next.

   Status get_page_no(const void *key, AttrType key_type, PageId & pageNo);

   bool get_sibling(const void *key, AttrType key_type,
                    PageId & pageNo, int &left);
    
// ------------------- Iterators ------------------------
// The two functions: get_first and get_next provide an
// iterator interface to the records on a BTIndexPage.
// get_first returns the first <key, pageNo> pair from the page,
// while get_next returns the next key on the page.
// These functions make calls to HFPage::firstRecord() and
// HFPage::nextRecord(), and split the flat record into its
// two components: namely, the key and pageNo.
// return NOMORERECS when there are no more pairs.

   Status get_first(RID& rid, void *key, PageId & pageNo);
   Status get_next (RID& rid, void *key, PageId & pageNo);

// ------------------- Left Link ------------------------
// You will recall that the index pages have a left-most
// pointer that is followed whenever the search key value
// is less than the least key value in the index node. The
// previous page pointer is used to implement the left link.

   PageId getLeftLink(void) { return getPrevPage(); }
   void   setLeftLink(PageId left) { setPrevPage(left); }

   Status adjust_key(const void *newKey, const  void *oldKey,
                     AttrType key_type);

   bool redistribute(BTIndexPage *pptr, BTIndexPage *parentPtr,
                     AttrType key_type,
                     int left, const void *deletedKey);
    
   Status findKey(void *key, void *entry, AttrType key_type);

   // The remaining functions of SortedPage are still visible.
};

#endif
