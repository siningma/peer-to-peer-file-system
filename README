Title: README for Final Project Part 2, Spring 2012

Data: 04/12/2012

I. File & Compile
	add a new file directory filesys into the submitted code. 
	Makefile is in servant, you can compile it under servant directory
	
	There are three folders under servant
	system, filesys and protocol

II. Design
	+ Architecture
		- All the messages are composed by factory pattern. This is an OOP implementation.
			class Msg in message.cpp and message.h is an abstract class. Specific message classes are inherited from this class
			pure virtual functions are used to parse payload data and get payload data to C-string. These two functions are implemented by specific message class 
			Add a new pure virtual function in part 2. send_file_payload is used for the message that needs to send a file out. Those messages that do not need to send files have no need to implement this.
					
		- The design for servant system
			From one node's point of view, there are three threads for the connection between two nodes. Client or Listening thread, Recv thread and Send thread. The socket descriptor (sockfd) is the same for three threads of one connection
			If one node needs to send out a message, it will push into dispatcher queue. The dispatcher dispatches message based on socket discriptor.  	
			The dispatcher will dispatch messages to corresponding send thread. 

	+ File System description
		- Use multimap for name, sha1 and keyword index data structure. The reason for multimap is that filename may be the same. Map in STL do not allow the same key
			For search, insert and delete operation of a node in multimap, it is O(log(n)).
			The format for three index files are the same as ini file. 
			name_index file: filename=index
		 	sha1_index file: sha1=index
		 	keyword_index file: bitvector=index
		 	The implementation of the cache record data structure is a list. In cacheList file, the format is the sequential order of this list. One index number per line
		 	Current index value is also in the file system for restarting the system without reset everything.
		 	A tempMeta directory is created for temp meta files that is temporarily saved for status response and search response message so that the message can be forwarded.
		 	The structure is as follows:
		 	$(HomeDir)
			  +- init_neighbor_list
			  +- kwrd_index
			  +- name_index
			  +- sha1_index
			  +- cacheList
			  +- index
			  +- servant.log
			  +- files   <= directory
				   +- 0.data
				   +- 0.meta
				   +- 0.pass (if this is the store owner)
				   +- 1.data
				   +- 1.meta
				   +- ... 
			  +- tempMeta   <= directory
			  		+- 0.tempMeta
			  		+- ...		   
		