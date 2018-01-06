# CS214-Project-2
Multi threaded sorter

## To Do
* Add a global linked list with movie arrays
* Add a dynamically allocated array with thread ids
* <strike>Replace forks with threads</strike>
	* Call new function and pass the individual file path
	* Lock global linked list
* Go through linked list and merge the arrays recursively
	* Every merge will be in a new thread merging two structs
* Then print the array once the linked list has only one entry
