/* 
 * class HeapFile
 * $Id: heapfile.h,v 1.1 1997/01/02 12:46:39 flisakow Exp $ 
 */

#ifndef _HEAPFILE_H
#define _HEAPFILE_H

#include "minirel.h"
#include "page.h"

//  This trival heapfile implementation is a simple linked
//  list of pages.
//
//  The first page is a header page for the entire database. 
//  (it is the one to which our filename is mapped by the DB).
//
//  See the file 'hfpage.h' for specifics on the page implementation.

      // Error codes for HEAPFILE.
enum heapErrCodes {
    BAD_RID,
    BAD_REC_PTR,
    HFILE_EOF,
    INVALID_UPDATE,
    NO_SPACE,
    NO_RECORDS,
    END_OF_PAGE,
    INVALID_SLOTNO,
    ALREADY_DELETED,
};

class HFPage;

class HeapFile {

  public:
      // Initialize.  A null name produces a temporary heapfile which will be
      // deleted by the destructor.  If the name already denotes a file, the
      // file is opened; otherwise, a new empty file is created.
    HeapFile( const char *name, Status& returnStatus ); 
   ~HeapFile();

      // return number of records in file
    int getRecCnt();
    
      // insert record into file
    Status insertRecord(char *recPtr, int recLen, RID& outRid); 
    
      // delete record from file
    Status deleteRecord(const RID& rid); 

      // updates the specified record in the heapfile.
    Status updateRecord(const RID& rid, char *recPtr, int reclen);

      // read record from file, returning pointer and length
    Status getRecord(const RID& rid, char *recPtr, int& recLen); 

      // initiate a sequential scan
    class Scan *openScan(Status& status);

      // delete the file from the database
    Status deleteFile();


  private:
    friend class Scan;

    enum Filetype {
        TEMP,
        ORDINARY
    };

    PageId      _firstPageId;    // page number of header page
    Filetype    _ftype;
    bool        _file_deleted;
    char       *_fileName;
};


#endif    // _HEAPFILE_H
