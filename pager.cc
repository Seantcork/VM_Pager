#include "vm_pager.h"
#include <assert.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <queue> 
#include <sys/types.h>
#include <iostream> 
#include <string>

/* Authors: Sean Cork & Dustin Hines
 *		scork@bowdoin.edu, dhines@bowdoin.edu
 * Program Details: This is an implementation of an external pager.
 * It which virtual pages are in virtual memory adn implements fucntions used
 * by applications. It implements the pager functioins of vm_extend() which
 * return the lowets valid part of the virtual arena. vm_fault() which handles
 * page falts repsectively. It also implemets vm_syslog which prints a message
 * of length len to the pager. It also implemets v_destroy that handles destoyed processes and 
 * vm_switch amd vm_create which the former switches processes and the later initializes 
 * the data structures necessary for a new process.
 */
using namespace std;

/*---------------------Structure Definitions----------------------*/
//primative declaration for use in the perPageInfo struct
struct perProcessInfo;

//sturct used to hold the information for all page entry's
struct perPageInfo { 
	perProcessInfo* myProcess;
	unsigned long virtualPage;
	unsigned long ppage; 
	int modified;
	int notZeroFilled; //check when reading a page
	int reference;
	int resident;
	//set to -1 if page isnt in disk
	unsigned long diskBlock; //keep track of disk block page is in
};

//this struct helps keep track of critical information per process 
struct perProcessInfo {
	//current pid
	pid_t myPid;
	page_table_t* pageTable; //pageTable[ppage] is the entry of page table we want
	//map from page number to additional info for the page 
	//page number is 64 bits 
	map<unsigned long, perPageInfo*> pageInfo; //virtual page 
	unsigned long nextVirtualADDR;
	unsigned long nextVirtualPage;
};

//keeps track of global processes
struct allProcessInfo {
	map<pid_t, perProcessInfo*> perProcessInfos;
	queue<perPageInfo*> clockStructure; //perPage info for each resident page in memory
};

//keeps track of number of disk blocks overall and the disk blocks currently free
struct diskInfo {
	unsigned long numberOfBlocks;
	queue<unsigned long> freeBlocks;
};

//keep track of the size of memeory and the free memory pages currently free
struct physMemInfo {
	unsigned long size; //number of physical memory pages 
	queue<unsigned long> freePages; 
};

/*---------------------Global Varibles----------------------*/
static allProcessInfo* globalProcessInfo;
static physMemInfo* globalMemInfo;
static diskInfo* globalDiskInfo; 
static perProcessInfo* currentProcessInfo;

/*---------------------Primative Definitions----------------------*/
//helper functions for vm_fault()
static int normalPageFault(map<unsigned long, perPageInfo*>::iterator itr,
	bool write_flag, unsigned long virtualPageNum);
static int notZeroFilledPageFault(map<unsigned long, perPageInfo*>::iterator itr, 
	bool write_flag, unsigned long virtualPageNum);
static int readWriteFault(map<unsigned long, perPageInfo*>::iterator itr, 
	bool write_flag, unsigned long virtualPageNum);
/*---------------------Function Definitions----------------------*/


//purpose: interate through the clock structre and evict the first page to 
//	have a reference bit of zero
//parameters: none
//return: returns the ppage of the now free memory page
static unsigned long secondChance() {
	//run second chance on the currentProcesses data structure, 
	while(globalProcessInfo->clockStructure.front()->reference == 1) {
		perPageInfo* curr = globalProcessInfo->clockStructure.front();
		//set w/r to 0, reference bit to 0
		curr->myProcess->pageTable->ptes[curr->virtualPage].read_enable = 0;
		curr->myProcess->pageTable->ptes[curr->virtualPage].write_enable = 0;
		curr->reference = 0;
		globalProcessInfo->clockStructure.pop();
		globalProcessInfo->clockStructure.push(curr);
	}
	//now the page on the front of the queue has a reference bit of 0
	perPageInfo* evicting = globalProcessInfo->clockStructure.front();
	//advance the clock hand
	globalProcessInfo->clockStructure.pop();
	
	if(evicting->modified == 1) {
		//write the page to disk
		disk_write(evicting->diskBlock, evicting->ppage);
	}
	//set the bits properly on eviction
	evicting->resident = 0;
	evicting->modified = 0;
	evicting->reference = 0;
	//set the read and write bits to zero in the page table 
	//we need to know which process this page belongs to
	evicting->myProcess->pageTable->ptes[evicting->virtualPage].read_enable = 0;	
	evicting->myProcess->pageTable->ptes[evicting->virtualPage].write_enable = 0;
	//update used memory pages and disk for the process
	return evicting->ppage;
}


//purpose: either get a free physical page and return it or run the clock algorithm
//Parameters: none
//return value: free phsiical page from memeory
static unsigned long getPhysPage() {
	// if there is already a free page
	if(globalMemInfo->freePages.size() > 0) {
		//use this free page
		unsigned long physMemPage = globalMemInfo->freePages.front();
		globalMemInfo->freePages.pop();
		return physMemPage;
	}
	//if there is not a free page
	else {
		unsigned long physMemPage = secondChance();
		return physMemPage;
	}
}

//purpose: initailize the data structures needed to run the 
//esternal pager program
//parameters: memory_pages, the number of phisical memeory pages and disk_blocks
//number of phisical disk blocks.
//return value: none.
void vm_init(unsigned memory_pages, unsigned disk_blocks) {
	
	//set up data structure to keep track of physical memory
	globalMemInfo = new physMemInfo;
	int numberPages = (int) memory_pages;
	globalMemInfo->size = numberPages;
	for(unsigned long i = 0; i < memory_pages; ++i) {
		globalMemInfo->freePages.push(i);
	}	
	//set up global structure to keep track of disk blocks
	globalDiskInfo = new diskInfo;
	int numberBlocks = (int) disk_blocks;
	globalDiskInfo->numberOfBlocks = numberBlocks;
	for(unsigned long i = 0; i < disk_blocks; ++i) {
		globalDiskInfo->freeBlocks.push(i);

	}
	//initialize structure to keep track of process information
	globalProcessInfo = new allProcessInfo;
	page_table_base_register = NULL;
}

//purpose: create data structures for process and make sure they are correct
//parameters: pid of new process created
//return value: nont
void vm_create(pid_t pid) {
	//make a new struct to store process info
	perProcessInfo *processInfo = new perProcessInfo;
	//create a new page table
	page_table_t *pageTable = new page_table_t;
	processInfo->pageTable = pageTable;
	processInfo->myPid = pid;
	processInfo->nextVirtualADDR = (unsigned long)VM_ARENA_BASEADDR;
	processInfo->nextVirtualPage = 0;
	globalProcessInfo->perProcessInfos.insert(make_pair(pid,processInfo));
}

//purpose: clean up pager state to account for a process ending
//parameters: none
//return: none
void vm_destroy() {
	pid_t currPid = currentProcessInfo->myPid;
  	unsigned startingSize = globalProcessInfo->clockStructure.size();
	//remove this processes' members of the clock structure 
	for(unsigned i = 0; i < startingSize; i++) {
		perPageInfo* curr = globalProcessInfo->clockStructure.front();
		globalProcessInfo->clockStructure.pop();
		//this member of the clockStructure does not belong to the 
		//process being destroyed
		if(curr->myProcess->myPid != currPid){
				globalProcessInfo->clockStructure.push(curr);
		}
	}
	map<unsigned long, perPageInfo*>::iterator iter;
	//free the global memory and disk resources used by process being
	//destroyed
	for(iter = currentProcessInfo->pageInfo.begin(); 
	    iter != currentProcessInfo->pageInfo.end(); iter++) {
		//if resident, page is now free
		if(iter->second->resident == 1) {
			globalMemInfo->freePages.push(iter->second->ppage);
		}
		//disk block is now free
		globalDiskInfo->freeBlocks.push(iter->second->diskBlock);
		//free the allocated perPageInfo struct
		perPageInfo* toFree = iter->second;
		delete toFree;
	}
	//this fixed an insane bug but shouldnt be necessary
	//	couldn't figure out a way around doing this...
	//	otherwise pages from previous processes will randomly show 
	//	up in the page tables of later processes
	for(unsigned i = 0; i < currentProcessInfo->nextVirtualPage; i++) {
		currentProcessInfo->pageTable->ptes[i].ppage = 0;
		currentProcessInfo->pageTable->ptes[i].read_enable = 0;
		currentProcessInfo->pageTable->ptes[i].write_enable = 0;
	}
	//free our data structures used to keep track of the info for the process
	page_table_base_register = NULL;
	delete currentProcessInfo->pageTable;
	//free allocated perProcessInfo struct
	map<pid_t, perProcessInfo*>::iterator itr = globalProcessInfo->perProcessInfos.find(currPid);
	delete itr->second;
	globalProcessInfo->perProcessInfos.erase(currPid);
}

//purppse: swtich to another process amd update relevant info
//Parameter: Pid of proces swe are switching it
//return value: none
void vm_switch(pid_t pid) {
	//find the page table for this pid
	map<pid_t, perProcessInfo*>::iterator itr = globalProcessInfo->perProcessInfos.find(pid);
	page_table_base_register = itr->second->pageTable;
	currentProcessInfo = itr->second;
	return;
}

//purpose: function that adresses read, write and page faults
//parameters: addr of faulting page, and write flag whcih indicates whether its a write fault
//or a read fault
//return: 0 on success, -1 if the address corresponds with an invalid page
int vm_fault(void* addr, bool write_flag) {
	unsigned long address = (unsigned long) addr;
	//need to determine which page this address corresponds to
	//	right shift by 13 to divide address by 8192 to get 
	//	the page number 
	unsigned long virtualPageNum = (address - (unsigned long)VM_ARENA_BASEADDR) / (unsigned long)VM_PAGESIZE;  
	map<unsigned long, perPageInfo*>::iterator itr;
	itr = currentProcessInfo->pageInfo.find(virtualPageNum);
	if(itr == currentProcessInfo->pageInfo.end()) {
		return -1;
	}
	//page is not resident
	if(itr->second->resident == 0) {
		//secondChance returns the page in physical memory that the page
		//was swapped into
		//if it's being read and it was extended and not zero filled
		if(itr->second->notZeroFilled) {
			return notZeroFilledPageFault(itr, write_flag, virtualPageNum);
		}
		//normal page fault
		else {
			return normalPageFault(itr, write_flag, virtualPageNum);
		}
	}
	//page is resident
	return readWriteFault(itr, write_flag, virtualPageNum);
}
//Purpose: Handles normal page fault scenario as determined by vm_fault
//Parameters: iterator pointing to the member of the virtual adress and 
//	perPageInfo map that is being faulted on, bool indicating whether it is 
//	a write fault, unsigned long for the virtual page number being faulted on
//Return: -1 on failure, 0 on success.
static int normalPageFault(map<unsigned long, perPageInfo*>::iterator itr, 
	bool write_flag, unsigned long virtualPageNum) {

	unsigned long physPage;
	physPage = getPhysPage();
	//read the disk info into the page we got
	disk_read(itr->second->diskBlock, physPage);
	//add the page information to the clock structure 
	if(!write_flag) {
		itr->second->modified = 0;
		currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 0;
	}
	else {
		itr->second->modified = 1;
		currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 1;
	}
	itr->second->resident = 1;
	itr->second->reference = 1;
	itr->second->ppage = physPage;
	currentProcessInfo->pageTable->ptes[virtualPageNum].ppage = physPage;
	currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
	//add this perpage info into the clock structure 
	globalProcessInfo->clockStructure.push(itr->second);      
	//  set the read/write bit in the actual page table...
	return 0;
}

//Purpose: Handles the special page fault scenario where we have defered work
//	by not zero filling a page that needs to present a zero filled view to the 
//	application.
//Parameters: iterator pointing to the member of the virtual adress and 
//	perPageInfo map that is being faulted on, bool indicating whether it is 
//	a write fault, unsigned long for the virtual page number being faulted on
//Return: -1 on failure, 0 on success.
static int notZeroFilledPageFault(map<unsigned long, perPageInfo*>::iterator itr, 
	bool write_flag, unsigned long virtualPageNum) {

	unsigned long physPage;
	physPage = getPhysPage();
	//zero fill this page in memory
	//first byte we want to be zero = 8192 * physPage
	//last byte we want to be zero = first byte + 8191
	unsigned long firstByte = physPage * VM_PAGESIZE;
	unsigned long lastByte = firstByte + VM_PAGESIZE - 1;
	for(unsigned long byte = firstByte; byte <= lastByte; byte++) {
		((char*) pm_physmem)[byte]= 0;
	}
	//set write and modified bits depending if a write/read
	if(!write_flag) {
		itr->second->modified = 0;
		currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 0;
	}
	else {
		itr->second->notZeroFilled = 0;
		itr->second->modified = 1;
		currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 1;
	}
	//always will read enable 
	currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
	itr->second->resident = 1;
	itr->second->reference = 1;
	itr->second->ppage = physPage;
	currentProcessInfo->pageTable->ptes[virtualPageNum].ppage = physPage;
	//add this perpage info into the clock structure 
	globalProcessInfo->clockStructure.push(itr->second);      
	//  set the read/write bit in the actual page table...
	return 0;
}

//Purpose: Function that handles non-page faults for the vm_fault function 
//Parameters: iterator pointing to the member of the virtual adress and 
//	perPageInfo map that is being faulted on, bool indicating whether it is 
//	a write fault, unsigned long for the virtual page number being faulted on
//Return: -1 on failure, 0 on success.
static int readWriteFault(map<unsigned long, perPageInfo*>::iterator itr, 
	bool write_flag, unsigned long virtualPageNum) {

	if(!itr->second->notZeroFilled) {

		if(write_flag) {
			//then the modified bit should be zero 
			//want the read and write bits to be 1, dont needs 
				//faults until its hit by second chance 
			itr->second->reference = 1;
			itr->second->modified = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
			return 0;
		}
		else {
			//want the write bit to be zero 
			//read bit to 1
			itr->second->reference = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
			if(itr->second->modified == 0) {
				currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 0;
			}
			else {
				currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 1;
			}	
			return 0;  
		}
	}
	//this is a resident page that has been extended but not modified
	//and it has not been written to disk
	else {
		if(write_flag) {
			itr->second->notZeroFilled = 0;
			itr->second->reference = 1;
			itr->second->modified = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].write_enable = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
			return 0;
		}
		else {
			itr->second->reference = 1;
			currentProcessInfo->pageTable->ptes[virtualPageNum].read_enable = 1;
			return 0;  
		}
	}
}

//Purpose: Declare as valid the lowest invalid virtual page in the current process's
// arena. Returns the lowest-numbered byte of the newly valid virtual page.
// Parameters: none
// return value: the lowest valid virual adress;
void* vm_extend() {
	//give the process another page in memory and another disk block
	
	//not enough space in the vitual address space to allocate anothe page
	if((unsigned long)currentProcessInfo->nextVirtualADDR + (unsigned long)VM_PAGESIZE > 
		(unsigned long)VM_ARENA_BASEADDR + (unsigned long)VM_ARENA_SIZE) {
		return NULL;
	}
	//no space on disk for additional pages
	if(globalDiskInfo->freeBlocks.size() == 0) {
		return NULL;
	}
	//There are free disk blocks and virautl adresses left
	unsigned long newVirtualADDR = currentProcessInfo->nextVirtualADDR; 
	currentProcessInfo->nextVirtualADDR = (unsigned long)currentProcessInfo->nextVirtualADDR
		+ (unsigned long)VM_PAGESIZE;
	unsigned long newPageNum = currentProcessInfo->nextVirtualPage;
	currentProcessInfo->nextVirtualPage ++;

	//create a new page table entry and set the bits appropriatly
	//and add it to the page table
	page_table_entry_t newEntry; 
	newEntry.read_enable = 0;
	newEntry.write_enable = 0;
	page_table_base_register->ptes[newPageNum] = newEntry;
	
	//create a enw struct to keep information for the page
	perPageInfo* newPerPageInfo = new perPageInfo;
	newPerPageInfo->myProcess = currentProcessInfo;
	//get a new disk block for this page
	newPerPageInfo->diskBlock = globalDiskInfo->freeBlocks.front();
	globalDiskInfo->freeBlocks.pop();
	//set the values for the page and add it to  
	newPerPageInfo->virtualPage = newPageNum;
	newPerPageInfo->notZeroFilled = 1;
	newPerPageInfo->modified = 0;
	newPerPageInfo->reference = 0;
	newPerPageInfo->resident = 0;  
	//add the extra page info to the map
	pair<unsigned long, perPageInfo*> addingExtraPageInfo (newPageNum, newPerPageInfo);
	currentProcessInfo->pageInfo.insert(addingExtraPageInfo);
	//retrun the lowets virtual adress
	return (void*) newVirtualADDR;
}

//Purpose: prints the messge of particular length len from the particular adres message
//parameters: message which is a virtual adress and len whcih is the length of the message
//return value: 0 on sucess -1 on failure
int vm_syslog(void* message, unsigned len) {
	string output;
	//address of message
	unsigned long vAddr = (unsigned long) message;
	map<unsigned long, perPageInfo*>::iterator itr;
	//if length of message is zero fail
 	if(len < 1) {
		return -1;
	}
	//ensure that the message starts in the arena
	if ((unsigned long)message < (unsigned long)VM_ARENA_BASEADDR) {
		return -1;
	}
	//ensure entire message is in arena
	if ((unsigned long)message + len > currentProcessInfo->nextVirtualADDR) {
		return -1;
	}
	//add chars to output
	for(unsigned i = 0; i < len; i ++) {
		unsigned long vPage = (vAddr - (unsigned long)VM_ARENA_BASEADDR) / (unsigned long)VM_PAGESIZE;		
		itr = currentProcessInfo->pageInfo.find(vPage);
		//virtual address in the arena
		perPageInfo* pageEntry = itr->second;
		//if the page isnt read enabled fault to treat it as a aplication read
		if(!page_table_base_register->ptes[pageEntry->virtualPage].read_enable) {
			if(vm_fault((void*)vAddr, 0) == -1) {
				return -1;
			}
		}
		//now we know the address is not invalid and the 
		//page is in physical memory
		unsigned long physPage = pageEntry->ppage;
		//mask everything besides the first 13 bits
		unsigned long offset = vAddr & 0x1fff;
		output += ((char*) pm_physmem)[physPage * VM_PAGESIZE + offset];
		vAddr++;
	}
	cout << "syslog \t\t\t" << output << endl;
	return 0;
}


