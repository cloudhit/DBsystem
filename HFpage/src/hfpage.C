#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "hfpage.h"
#include "heapfile.h"
#include "buf.h"
#include "db.h"


// **********************************************************
// page class constructor

void HFPage::init(PageId pageNo)
{
    slotCnt = 0;
    usedPtr = MAX_SPACE - DPFIXED;
    freeSpace = MAX_SPACE - DPFIXED + sizeof(slot_t);
    prevPage = nextPage = INVALID_PAGE;
    curPage = pageNo;
}

// **********************************************************
// dump page utlity
void HFPage::dumpPage()
{
    int i;

    cout << "dumpPage, this: " << this << endl;
    cout << "curPage= " << curPage << ", nextPage=" << nextPage << endl;
    cout << "usedPtr=" << usedPtr << ",  freeSpace=" << freeSpace
         << ", slotCnt=" << slotCnt << endl;
   
    for (i=0; i < slotCnt; i++) {
        cout << "slot["<< i <<"].offset=" << slot[i].offset
             << ", slot["<< i << "].length=" << slot[i].length << endl;
    }
}

// **********************************************************
PageId HFPage::getPrevPage()
{
    return prevPage;
}

// **********************************************************
void HFPage::setPrevPage(PageId pageNo)
{
    prevPage = pageNo;
}

// **********************************************************
void HFPage::setNextPage(PageId pageNo)
{
    nextPage = pageNo;
}

// **********************************************************
PageId HFPage::getNextPage()
{
    return nextPage;
}

// **********************************************************
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns DONE if sufficient space does not exist
// RID of the new record is returned via rid parameter.
Status HFPage::insertRecord(char* recPtr, int recLen, RID& rid)
{
    int slotNo;
    if(freeSpace >= recLen){
        bool found = false;
        for(int i = 0; i < slotCnt; i ++){
            if(slot[i].length == - 1){
                slot[i].offset = usedPtr - recLen;
                slot[i].length = recLen;
                usedPtr -= recLen;
                freeSpace -= recLen;
                slotNo = i;
                found = true;
                break;
            }
        }
        if(!found){
            if(sizeof(slot_t) + recLen > freeSpace) return DONE;
            slotNo = slotCnt ++;
            freeSpace -= (sizeof(slot_t) + recLen);
            slot[slotCnt - 1].offset = usedPtr - recLen;
            slot[slotCnt - 1].length = recLen;
            usedPtr -= recLen;
        }
    }
    else return DONE;
    rid.pageNo = curPage;
    rid.slotNo = slotNo;
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
Status HFPage::deleteRecord(const RID& rid)
{
    if(rid.pageNo != curPage || rid.slotNo >= slotCnt || slot[rid.slotNo].length == -1 || slot[rid.slotNo].offset ==  -1)
        return FAIL;
    int no = rid.slotNo;
    for(int i = slot[no].offset - 1; i >= usedPtr; i --)
        data[i + slot[no].length] = data[i];
    usedPtr += slot[no].length;
    freeSpace += slot[no].length;
    for(int i = no + 1; i < slotCnt; i ++)
        slot[i].offset += slot[no].length;
    slot[no].offset = slot[no].length = -1;
    if(no == slotCnt - 1){
        freeSpace += sizeof(slot_t);
        slotCnt --;
    }
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID& firstRid)
{
    if(usedPtr == MAX_SPACE - DPFIXED) return DONE;
    for(int i = 0; i < slotCnt; i ++){
        if(slot[i].offset + slot[i].length == MAX_SPACE - DPFIXED){
            firstRid.pageNo = curPage;
            firstRid.slotNo = i;
            break;
        }
    }
    return OK;
}

// **********************************************************
// returns RID of next record on the page
// returns DONE if no more records exist on the page; otherwise OK
Status HFPage::nextRecord (RID curRid, RID& nextRid)
{
    int no = curRid.slotNo;
    if(curRid.pageNo != curPage || no >= slotCnt || slot[no].length == -1 || slot[no].offset == -1)
        return FAIL;
    if(slot[no].offset == usedPtr) return DONE;
    for(int i = 0; i < slotCnt; i ++){
        if(slot[i].offset + slot[i].length == slot[no].offset){
            nextRid.pageNo = curPage;
            nextRid.slotNo = i;
            break;
        }
    }
    return OK;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char* recPtr, int& recLen)
{
    int no = rid.slotNo;
    if(rid.pageNo != curPage || no >= slotCnt || slot[no].length == -1 || slot[no].offset == -1) 
        return FAIL;
    recLen = slot[no].length;
    for(int i = slot[no].offset; i < slot[no].offset + recLen; i ++)
        recPtr[i - slot[no].offset] = data[i]; 
    return OK;
}

// **********************************************************
// returns length and pointer to record with RID rid.  The difference
// between this and getRecord is that getRecord copies out the record
// into recPtr, while this function returns a pointer to the record
// in recPtr.
Status HFPage::returnRecord(RID rid, char*& recPtr, int& recLen)
{
    int no = rid.slotNo;
    if(rid.pageNo != curPage || no >= slotCnt || slot[no].length == -1 || slot[no].offset == -1) 
        return FAIL;
    recLen = slot[no].length;
    recPtr = &data[slot[no].offset];    
    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void)
{
    int remain = freeSpace;
    bool found = false;
    for(int i = 0; i < slotCnt; i ++){
        if(slot[i].length == -1 || slot[i].offset == -1){
            found = true;
            break;
        }
    }
    if(!found) remain -= sizeof(slot_t);
    return remain;
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void)
{
    for(int i = 0; i < slotCnt; i ++){
        if(slot[i].length != -1 && slot[i].offset != -1)
            return false;
    }
    return true;
}



