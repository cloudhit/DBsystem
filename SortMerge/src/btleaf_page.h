/* -*- C++ -*- */
/*
 * btleaf_page.h - definition of class BTLeafPage for Mini{base|rel} project.
 *
 */

#ifndef BTLEAF_PAGE_H
#define BTLEAF_PAGE_H

#include "minirel.h"
#include "page.h"
#include "sorted_page.h"
#include "bt.h"
#include "btindex_page.h"


/*
 * A BTLeafPage is a leaf page on a B+ tree.  It holds abstract 
 * <key, RID> pairs; it doesn't know anything about the keys 
 * (their lengths or their types), instead relying on the abstract
 * interface consisting of keyCompare(), make_entry(), get_key_data(),
 * and get_key[_data]_len() from bt.h + key.cc.  
 * See those files for our <key,data> pairing interface and implementation.
 *
 * Methods that were part of the original specification are commented
 * using //-style comments; original comments are left intact.
 * New methods added by use are commented with / * * /-style comments; these
 * appear at the end of the class definition.
 */
 

class BTLeafPage : public SortedPage {
  
 private:
   // No private variables should be declared.

 public:

/*
 * Error handling infrastructure, added by us:
 */

  enum ErrorTypes { 
    _OK = 0,
    LEAFINSERTRECFAILED,   /* these are indices */
    LEAFNR_ERRORS              /* and this is the number of them */
  }; 

  static const char* Errors[LEAFNR_ERRORS];


 private:
   // No private variables should be declared.

 public:

// In addition to initializing the  slot directory and internal structure
// of the HFPage, this function sets up the type of the record page.

   void init(PageId pageNo)
   { HFPage::init(pageNo); set_type(LEAF); }

// ------------------- insertRec ------------------------
   // READ THIS DESCRIPTION CAREFULLY. THERE ARE TWO RIDs
   // WHICH MEAN TWO DIFFERENT THINGS.
// Inserts a key, rid value into the leaf node. This is
// accomplished by a call to SortedPage::insertRecord()
// The function also sets up the recPtr field for the call
// to SortedPage::insertRecord() 
//
// Parameters:
//   o key - the key value of the data record.
//
//   o key_type - the type of the key.
//
//   o dataRid - the rid of the data record. This is
//               stored on the leaf page along with the
//               corresponding key value.
//
//   o rid - the rid of the inserted leaf record data entry,
//           i.e., the <key, dataRid> pair.
   
   Status insertRec(const void *key, AttrType key_type, RID dataRid, RID& rid);


// ------------------- Iterators ------------------------
// The two functions: get_first and get_next provide an
// iterator interface to the records on a BTLeafPage.
// get_first returns the first <key, RID> from the page,
// while get_next returns the next pair on the page.
// These functions make calls to HFPage::firstRecord() and
// HFPage::nextRecord(), and split the flat record into its
// two components: namely, the key and dataRid.
// Should return NOMORERECS when ther are no more pairs.

  Status get_first(RID& rid, void *key, RID & dataRid);
  Status get_next (RID& rid, void *key, RID & dataRid);

  /*
   * get_current returns the current record in the iteration; it is like
   * get_next except it does not advance the iterator.
   */

  Status get_current (RID rid, void *key, RID & dataRid);

// ------------------- get_data_rid ------------------------
// This function performs a sequential search (or a binary search
// if you are ambitious) to find a data entry of the form <key, dataRid>,
// where key is given in the call.  It returns the dataRid component
// of the pair; note that this is the rid of the DATA record, and
// NOT the rid of the data entry!

// NOT being used (through it is written).
// Status get_data_rid(void *key, AttrType attrtype, RID & dataRid);


  /* 
   * delUserRid -- delete a data entry with key `key' (of type key_type) and 
   * data-rid `userRid'.  Used by BTreeFile::Delete and
   * BTreeFileScan::delete_current().  (They know only the data entry RID
   * not the internal page RID; this is a convenience function for them.)
   */

  bool delUserRid (const void *key, AttrType key_type, const RID& dataRid);

  bool redistribute (BTLeafPage* page, BTIndexPage* parentpage,
          AttrType key_type, int left, const void *deleted_key);
    
};

#endif
