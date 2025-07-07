Program takes the command line inputs of format:	()=OPTIONAL

./project2 SORTMESIZE THRESHOLD (SEED) (MULTITHREAD) (PIECES) (MAXTHREAD)
  
The program takes this input, and creates an array of size SortMeSize. It then populates the array from 0 to (SortMeSize-1). It then scrambles the array so the values are in a random order. From there, depending on Multithread on if Multithread is on, it'll segment the array into Pieces partitions, seperated by pivot values. 
All values to the left of this pivot are less than it (though not neccesarily sorted) and all values to the right of the pivot are greater than the pivot. From there, we begin a multithreaded quicksort. The program creates Maxthreads threads at first, with each thread tasked in sorting a partition.
After that, it waits until 1 thread is complete to create the next one, until we've sorted Pieces threads. Then we double check that our array is sorted, and output some information about the runtime

The way the sorting works is it's a quicksort-shellsort hybrid. If the size of the partition is greater than the Threshold value, we use quicksort. If the value is less than the Threshold value, we use Shell Sort. The way quicksort works is similar basically the same as how we partition the array above. We take a pivot value, 
and then go through the partition value by value, swapping them when we find a two values, one greater than the pivot to the left of where the pivot will go, and one less than the pivot to the right of where the pivot will go. Then we swap the values, and continue sorting. Once we complete this, we put the pivot in it's correct location, 
and recursively call Quicksort on the two new partitions on the pivot's left and right. The shellsort is explained below where thefunction for it is.
