1. Yupeng Zhang
2. My submitted solution can handle all the errors. 
3. My implementation of Love/Hate policy with a double linked list, a set, a vector:
Data Sturcture:
(1)I implemented LRU and MRU using a double linked list, and they are in the same list. Each node in the MRU/LRU list is in a continuous memory space actually given the frame_id, hence we just need to search target page’s frame_id using built hash table which is just the index of its corresponding node. With this structure, all operations needed including insertion, deletion and searching cost only O(1) time;
(2)Only page in the buffer with “pin_count = 0” can exist in the list. When its count becomes larger than 0 later or it is freed, just delete it from the list;
(3)Used a set to store frame_id which has a loved page. Just check whether a page was loved ever using the set to determine whether to add it to the MRU/LRU list. And delete it from the set when the page is freed;
(4)Used a vector to store free frame_id as a queue, if it’s empty, no free frame exists and then if list is empty, no frame can be replaced;
Operation:
(1)If a page in buffer is unpinned with “hate = true”, first check the set and then search it in the list and delete it if exists, then the node with its info will be added to the list head, while with “hate = false”, delete it from the list if exists before and it will be added to both the list end and the set. Then when a replacement happens, just replace the page at the list head;




