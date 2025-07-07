
//Tony Pellican
//EECS 3540:Operating Systems and Systems Programming
//Dr. Thomas
//3-8-2021


//Before we begin looking at the program, if you're using vim, enter the command :set tabstop=4
//It'll make everything easier to read and the "diagrams" will make more sense.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

//Program takes the command line inputs of format:	()=OPTIONAL
//		./project2 SORTMESIZE THRESHOLD (SEED) (MULTITHREAD) (PIECES) (MAXTHREAD)
//The program takes this input, and creates an array of size SortMeSize. It then populates the array from 0 to (SortMeSize-1). It then scrambles the array so the values are in a random order. From there, depending on Multithread on if Multithread is on, it'll segment the array into Pieces partitions, seperated by pivot values. All values to the left of this pivot are less than it (though not neccesarily sorted) and all values to the right of the pivot are greater than the pivot. From there, we begin a multithreaded quicksort. The program creates Maxthreads threads at first, with each thread tasked in sorting a partition. After that, it waits until 1 thread is complete to create the next one, until we've sorted Pieces threads. Then we double check that our array is sorted, and output some information about the runtime
//The way the sorting works is it's a quicksort-shellsort hybrid. If the size of the partition is greater than the Threshold value, we use quicksort. If the value is less than the Threshold value, we use Shell Sort. The way quicksort works is similar basically the same as how we partition the array above. We take a pivot value, and then go through the partition value by value, swapping them when we find a two values, one greater than the pivot to the left of where the pivot will go, and one less than the pivot to the right of where the pivot will go. Then we swap the values, and continue sorting. Once we complete this, we put the pivot in it's correct location, and recursively call Quicksort on the two new partitions on the pivot's left and right. The shellsort is explained below where thefunction for it is.

int *SortMe=NULL;	//Pointer to array of numbers to be sorted (the point of the project)
int Threshold;		//Threshold value for determining when a sort should switch from quicksort to shellsort.

//struct used for passing data to a thread for quicksorting
typedef struct 
{
	int start;
	int end;
} QParameters;

bool IsSorted(int Size)
//Function to determine in an array is sorted. Goes through the full array element by element and sees if the next element is bigger than the previous element, if so, returns false, since the array is not sorted. If no element is bigger than the element directly to the right of it, return true.
{
for(int i=0; i< Size-1; i++)	//Goes from the first element to the second to last element
{
	if(SortMe[i] > SortMe[i+1])	//If the given element is bigger than the next element
	{
		return false;		//function returns false
	}
}
//If we get through the array and no element is bigger than the next element, the array is sorted.
return true;
}

//--------------------------------------------------------------------------------------------------Function for using ShellSort to sort an array.Goes through partition of the array with a "comb" and sorts the values at each "tooth", then shortens the distance between each"tooth" and sorts again. continues until the distance is one, and after checking the partition returns. See below for example.	5 6 8 3 4 2 7 9 1--->3 6 8 5 4 2 7 9 1	3 6 8 5 4 2 7 9 1------>3 4 8 5 6 2 7 9 1	   			    ^     ^     ^        ^     ^     ^	      ^     ^     ^           ^     ^     ^  					3 4 8 5 6 2 7 9 1------>3 4 1 5 6 2 7 9 8	3 4 1 5 6 2 7 9 8------>1 4 3 5 6 2 7 9 8                   ^     ^     ^           ^     ^     ^   ^   ^   ^   ^   ^	    ^   ^   ^   ^   ^				1 4 3 5 6 2 7 9 8------>1 2 3 4 6 5 7 9 8	1 2 3 4 6 5 7 9 8------>1 2 3 4 5 6 7 8 9	 	          ^   ^   ^   ^           ^   ^   ^   ^     ^ ^ ^ ^ ^ ^ ^ ^ ^	    ^ ^ ^ ^ ^ ^ ^ ^ ^

void ShellSort(int start, int end)
//Function for Shell Sorting an array
{
int k=1;			//Value used to keep track of the swapping distance
int N=end-start+1;	//Size of the partition to be sorted
int temp;			//place to store a variable while swapping

while(k<=N){k*=2;}	//Used to get the real initial power of K by going through the powers of 2
k=(k/2)-1;			//once we've found a power of 2 greater than the array size, go back down a
					//level (k/2) and subtract 1 
do	//While our swapping distance is greatrer than 0.
{
	for(int i=start;i < (end-k+1);i++)		//For each of the comb positions
	{
		for(int j=i;j >=start; j-=k)		//Go through each of the teeth in the comb.
		{
			if(SortMe[j] <= SortMe[j+k])	//Is the values shouldn't be swaped
			{
			break;							//Go the next comb position
			}
			else
			{
			temp=SortMe[j];					//swap SortMe[j] and SortMe[j+k]
			SortMe[j]=SortMe[j+k];
			SortMe[j+k]=temp;
			}
		}
	}
k=k>>1;	//Fancy divide by 2.
}
while(k>0);	//end of do while
return;
}

//--------------------------------------------------------------------------------------------------Used the Partition the array in the Partitions section of main, takes the start and end positions of a partition, and sorts them similar to Quicksort, see the second half of quicksort below. returns the position of the pivot value in the partition

int FPartition(int start, int end)
{
int temp=-5;			//temp value used to temporarily store values while we swap.
int pivot=SortMe[start];//our pivot value is the left most value in the partition
int i=start;			//start at the two ends of the partition with an i and j "pointer"
int j=end+1;
if(start >= end){return end;}	//if the partition is size 1 or less, return our end position.Likely means the array is too small for the number of pieces needed.

do{							//Go through loop until we hit break
	do i++; while(SortMe[i] < pivot);	//Go from right to left until we find a value < pivo
	do j--; while(SortMe[j] > pivot);	//Go from left to right until we find a value > pivo
	if( i < j)							//if our i and j pointers haven't "crossed" yet
	{
		temp=SortMe[i];					//swap the i and j value (since i>pivot>j) and we
		SortMe[i]=SortMe[j];			//want i<pivot<j for in order)
		SortMe[j]=temp;	
	}
		else break;						//if i and j did cross, we're don going through the 
}while(true);				//partition for this iteration

							//swap the start value (our pivot) and the jth value so 
SortMe[start]=SortMe[j];	// all values <pivot are on pivot's left and all values >pivot are
SortMe[j]=pivot;			//on the right of pivot(not necessarily in order.)

return j;	//return the position of the povit value.
}

//--------------------------------------------------------------------------------------------------The Quicksort function can be "divided" into 2 halves. The first half checks for use cases where Quicksort wouldn't be the best approach (ie if the partition we're sorting only contains 1 or 2 elements.) It also then checks to see if we've passed the Threshold value, and if we have we call ShellSort                                                                                                    If none of those conditions are met, we go into Quicksort, where we find a "pivot" value (we just use the value on the right side of the partition) and then swap numbers in the partition based on value relative to quicksort (< or >). Then we swap in our pivot value so the in the partition, all values to the left of our pivot value are less than the pivot value and all the numbers to the right of the pivot are greater than the pivot value. Then we recursively call Quicksort again on the new left and right partitions,  until we meet one of the edge cases outlined in the first half.


void Quicksort(int start, int end)
{
int temp=-5;			//temp value used as a place holder while swapping values in the partition
if(start>=end){return;}	//1 or fewer values in the partition no need to continue
if(end-start+1 == 2 )	//If the partition only has 2 values
{
	if(SortMe[start] > SortMe[end])		//check to see if the left value is bigger than the
	{									//right value
		temp=SortMe[start];				//if so swap them
		SortMe[start]=SortMe[end];
		SortMe[end]=temp;
	}
	return;								//end the function regardless of swapping
}
if(end-start+1 <= Threshold)
{
ShellSort(start, end);
return;
}

int pivot=SortMe[start];//our pivot value is the left most value in the partition
int i=start;			//start at the two ends of the partition with an i and j "pointer"
int j=end+1;


do{						//Go through loop until we hit break
	do i++; while(SortMe[i] < pivot);	//Go from right to left until we find a value < pivo
	do j--; while(SortMe[j] > pivot);	//Go from left to right until we find a value > pivo
	if( i < j)							//if our i and j pointers haven't "crossed" yet
	{
		temp=SortMe[i];					//swap the i and j value (since i>pivot>j) and we
		SortMe[i]=SortMe[j];			//want i<pivot<j for in order)
		SortMe[j]=temp;	
	}
		else break;						//if i and j did cross, we're don going through the 
}while(true);							//partition for this iteration

							//swap the start value (our pivot) and the jth value so we're
SortMe[start]=SortMe[j];	// all values <pivot are on pivot's left and all values >pivot are
SortMe[j]=pivot;			//on the right of pivot(not necessarily in order.)

Quicksort(start,j-1);	//recursively call quicksort on the new left partition
Quicksort(j+1,end);	//and on the right partition
return;		//After we've quicksorted both sides, we know this whole partition is sorted
}

//--------------------------------------------------------------------------------------------------Sort it helper function-Used for parsing data for Quicksort. Called upoon thread creation, takes the void struct pthread_create and casts it back to the struct we have at the top of the program. We then parse the data out of it we need and use that to call Quicksort.Upoon Quicksort's completion, we exit the thread used to call this function.

void *SortIt(void* RecievedData)
{
	QParameters *QData= (QParameters*)RecievedData;	//Casts Recieved data to the struct we have
							// already made
	Quicksort(QData->start,QData->end);		//Calls quicksort using the data from the 
							//struct

	pthread_exit(0);				//exit the thread.
}


//--------------------------------------------------------------------------------------------------main is fairly large, and so is "broken" up into sections, this first section really only creates all the variables, parses a couple of them from required command line inputs, and starts the timer used to measure the elapsed time of the whole program.

int main(int argc,char *argv[])
{
int SortMeSize=atoi(argv[1]);//Size of List to be sorted
Threshold=atoi(argv[2]);	//The Threshold Size (see explanation above)
int Seed=1;					//Value we seed array with (only used for output purposes.)
bool Multithread = true;	//Should we Multithread
int Pieces=10;				//How many partitions should we make
int Maxthreads=4;			//Max number of threads we can run (4 for 4 cores, which is defualt)
struct timeval WallStart;	//Time we start sorting
struct timeval WallEnd;		//Time we finish sorting
clock_t CPUStart;			//CPU time we start sorting at
clock_t CPUEnd;				//CPU time we stop sorting at
float CreateTime;			//CPU clock time taken to create the array
float InitializeTime;		//CPU clock time taken to initialize the array
float ShuffleTime;			//CPU clock time taken to shuffle the array
float PartitionTime;		//CPU clock time taken to Partition the array
float SortWallTime;			//used to store Wall Clock time taken to sort
float TotalWallTime;		//Used to store Wall Clock time taken to do the whole program
int random;					//random value used in randomizing array
int temp;					//temporary place to hold a value while rearraging array
int i,j;					//For going through for loops (its i and j, what did you expect.)
int pivot;					//For sotring the values we partitioned at
clock_t TotalTime=clock();	//For getting the total elapsed time in terms of the CPU clock
struct timeval TotalStart;	//The Wall clock time we started the program at
struct timeval TotalEnd;	//The Wall Clock time we finished the program at
int max;					//For keeping track of the largest Partitioni
bool MaxThreadsRunning;		//Used to keep track if we have MaxThreads threads running
unsigned int WaitTime=50;	//Time we wait while checking to see if a thread has finished
int ThreadsMade=0;			//Thread that have been launched
int FinishedThread;	
//struct used for transfering data to the threads.

gettimeofday(&TotalStart,NULL);

//Going through and updating values based on optional inputs(becuase the presence of the first arguement is required for the second arguement and so on, using nested ifs as oppposed to switches.)
//--------------------------------------------------------------------------------------------------Parsing Command line inputs-Goes through and assigns values to variables based on command line inputs.Uses nested if, since if one of the optional inputs is missing, then there should be no more inputs after that missing one (so a command line with no seed shouldn't have a Multithread or pieces arguement either.)Then goes through and checks the validity of some of the inputs, like if number of threads we run(MaxThreads) is greater than pieces(the number of partitions we run on speerate threads.)

if(argc>3)				//If a seed arguement is present
{
	Seed=atoi(argv[3]);	//Save the arguement to Seed
	if(Seed==-1)		//If the seed is -1
	{
		srand(clock());	//srand is seeded by the clock value (random)
	}
	else				//else if the see is not -1
	{
		srand(Seed);	//srand is seeded with the input value
	}
if(argc>4)						//If there is a Multithread arguement
{
	if((strcmp(argv[4],"n")==0)||(strcmp(argv[4],"N")==0))
	{							//If the Multithread arguement is n or N
		Multithread=false;		//set Multithread to false (no multithreading)
		Maxthreads=1;			//If we're not Muultithreading, there's only 1 thread
		Pieces=1;
	}
if((argc>5)&&(Multithread!=false))	//If there is a Pieces arguement and if Multithread is false
{
	Pieces=atoi(argv[5]);			//set the number of partitions(Q-Sort) to be argv 5 
	
if((argc>6)&&(Multithread != false))	//If there is a Maxthreads argument and Multithread isn't fa
{
	Maxthreads=atoi(argv[6]);			//set the max # of threads we can run to argv 6
}}}}
if(Maxthreads > Pieces)			//Checking to see if Pieces is >= Max thread
{
printf("Error:Pieces must be > or = to Maxthread.\n");	//If it is,print an error message and exit
return 1;
}
if(Pieces<=1){Multithread=false;}	//It's not Mulitthreading with 1 ro fewer piece, have the 
									//mulithread input be n if you want multithread off.

//--------------------------------------------------------------------------------------------------Populating the array-Actually does two things,initialize the array of size SortMeSize and then fills it with values 0-(SortMeSize-1). So a SortMeSize of 500 would have an array of 0-499.

CPUStart=clock();								//get current time.
SortMe=(int*)malloc(SortMeSize*sizeof(int));	//creates an array of size SortMeSize and puts a pointer to it in SortMe.
CreateTime=(float)(clock()-CPUStart)/CLOCKS_PER_SEC;//reports the time taken to initalize the array

CPUStart=clock();								//Time we start populating the array

for(i=0; i < SortMeSize;i++)	//Go through each spot in the array
{
	SortMe[i]=i;				//And put a random value in it
}

CPUEnd=clock();	//Done initalizing the array, so record the clock() value
InitializeTime=(float)(CPUEnd-CPUStart)/CLOCKS_PER_SEC;
//--------------------------------------------------------------------------------------------------Scrambling the array-scrambles the array so the values are no longer in numerical order. Goes through the array int by int and swaps it with another int at a random position in the array.

for(i=0; i < SortMeSize; i++)	//Goes through the array,element by element and
{
	random=rand()%SortMeSize;	//generates a random value between 0 and the size of the array
	temp=SortMe[random];		//and store the value of the "random"th value in the array in temp
	SortMe[random]=SortMe[i];	//sets the value of the "random"th spot in the array to be
								//equal to the ith value in the array
	SortMe[i]=temp;				//and finally sets the ith value in the array to be temp 0
								//(previously the "random"th value.)
}
ShuffleTime=(float)(clock()-CPUEnd)/CLOCKS_PER_SEC;

//--------------------------------------------------------------------------------------------------Patitioning the array-Creates Pieces partitions to be sorted on seperate threads later. First creates a 2D array called Partitions with Pieces rows and 3 columns. The 1st column houses the start position of the partition, the 2nd column houses the end psoition of the partition, and the third column houses the size of the partition. We then see if Multithreading is on.                                                                                                                                  If so, we put the whole array in as the first Partition in our array. We then go into a loop to create all the partitions. The loop takes the largest partition andcalls the partition function on it, which returns the pivot position. We then store the values for the left partition where we previously had the largest partition, and we store the values for the right partition in the next open spot in the Partitions array.We repeat until Partitions is full.                                                                                                                                               If not, we basically skip this section.

gettimeofday(&WallStart,NULL);	//getting the times for when we start sorting
CPUStart=clock();				//get the time we start doing the partitions at
int Partitions[Pieces][3]; 		//allocating an array for the Partitions. 1 piece is the left most
								//value of a partition.2 is the right most value of a partition.
								//3 is the size of the partition.

if(Multithread == true)		//if we aren't multithreading, no need to bother with partitions
{
	Partitions[0][0]=0;		//seting up the whole array in the first row of Partitions.
	Partitions[0][1]=SortMeSize-1;
	Partitions[0][2]=SortMeSize;

	for(i=1; i<Pieces;i++)//For going through to make (Pieces) seperate Partitions
	//Goes through with i pointing to the next row in Partition to be filled finds the row with 
	//the highest segment value (colmun 3) and partitions it. replaces the previous large 
	//partition with the new left partition and the ith row with the right partition. Repeat 
	//until  i <Pieces (note i needs to point to the next row we haven't created yet, so i 
	//starts at 1 and goes to Pieces + 1)
	{
		max=0;		//Default row with largest partition is the first row

		for(j=0;j<i;j++)	//Goes through the partitions we've made so far
		{					//and finds the largest partition and stores it's row value
							//in max
			if(Partitions[j][2] > Partitions[max][2])
			{
				max=j;
			}
		}
		//Sort the largest partition into two smaller partitions and return the pivot value
		pivot=FPartition(Partitions[max][0],Partitions[max][1]);
		
										//ith row is the right half of the partition
		Partitions[i][0]=pivot+1;		//Partition in ith row begins at pivot+1
		Partitions[i][1]=Partitions[max][1];	//Patition in ith row ends at partition  
												//max's old end value
		Partitions[i][2]=Partitions[max][1]-pivot+2;//segment length of new partition

		//maxth row is now the left half of the new partition (note Partition[max][0] does
		//not change
		Partitions[max][1]=pivot-1;	//Partition in max row ends at pivot-1
		Partitions[max][2]=pivot-Partitions[max][0];	//new partition segment
	}
}
PartitionTime=(float)(clock()-CPUStart)/CLOCKS_PER_SEC;	//print out the time it takes to partition


//--------------------------------------------------------------------------------------------------Sorting the array-Can be seperated into 3 parts.First, if Multithread is off, it just launches a quicksort in the same thread for the full array, nothing fancy.
//If the Multithread is on. then it begins by creating Maxthreads threads out of the 4 largest partitions. It then waits for 1 (or more) of those to finish. It then creates a new thread with another partition to sort and waits for another thread to be finished. After it has created Pieces partitions, it waits for all the threads to finish.
if(Multithread == true)
{
	pthread_t tid[Pieces];			//(Thread IDs for MaxThreads) threads
	pthread_t ActiveTID[Maxthreads];//Threads that are actively running
	pthread_attr_t attr;			//Default thread attributes
	pthread_attr_init(&attr);
	QParameters *QData;
	QParameters QDatas[Pieces];
	for(i=0; i < Maxthreads;i++)
	//Going through and making (Maxthreads) threads(default 4.)
	{
		max=0;			//Default largest thread is 0th thread
		for(j=0; j <Pieces;j++)		//Go through the Partitions array
		{							//to find the largest partition
			if(Partitions[max][2]<Partitions[j][2])
			{
				max=j;
			}
		}
		QDatas[i].start=Partitions[max][0];	//Set the data for the nex thread to be the
		QDatas[i].end=Partitions[max][1];		//the largest partition
		QData=&QDatas[i];
		pthread_create(&tid[i],&attr,SortIt,(void*)QData);	//Launch new thread
		printf("(%9i, %9i, %9i)\n",QDatas[i].start,QDatas[i].end,Partitions[max][2]);
		ActiveTID[i]=tid[i];				//adding the id of the thread just launched to active
											//threads
		ThreadsMade++;			//increase the count of how many threads have been made.
		Partitions[max][2]=0;	//set the length of the partition to be 0, so we don't launch 
								//another thread for that partition
	}



	for(i=Maxthreads; i < (Pieces);i++)
	//For going through and making Pieces-Maxthreads threads
	{
		max=0;			//Default largest thread is 0th thread
		for(j=0; j <Pieces;j++)		//Go through the Partitions array
		{				//to find the largest partition
			if(Partitions[max][2]<Partitions[j][2])
			{
				max=j;
			}
		}	
		QDatas[i].start=Partitions[max][0];	//setting the data for the next thread to be launched
		QDatas[i].end=Partitions[max][1];

		MaxThreadsRunning=true;	//set to true so we can keep going through the loop while 
								//there are no threads available

		while(MaxThreadsRunning==true)	//While all the threads we can run are running
		{
			for(j=0;j<Maxthreads;j++)	//For each of the threads we have made
			{
				if(pthread_tryjoin_np(ActiveTID[j],NULL)!=EBUSY)//if one of the active threads is no
				{												//longer running
					MaxThreadsRunning=false;	//set the bool we use to see if have the max threads
												//running to false
					FinishedThread=j;			//record which thread in the active array stopped.
				}
			}
			if(MaxThreadsRunning)		//If no thread was found to be not running
			{
				usleep(WaitTime);		//wait for 50ms and check again
			}							//else we leave the while loop
		}	
		QData=&QDatas[i];
		pthread_create(&tid[i],&attr,SortIt,(void*)QData);//launch a new thread with the partitio
															//data we found earlier.
		printf("(%9i, %9i, %9i)\n",QDatas[i].start,QDatas[i].end,Partitions[max][2]);
		ThreadsMade++;				//increment the number of threads we've made.
		ActiveTID[FinishedThread]=tid[i];	//replace the finished thread with the new one on the 
											//list of active threads
		Partitions[max][2]=0;		//set the partition's size to 0 so we don't use it again.
	}
	
	for(i=0;i<Pieces;i++)	//For all the threads we've ran
	{
	pthread_join(tid[i],NULL);	//check to see if they finished, and if not wait on them to finish.
	}
}
else
{
	Quicksort(0,SortMeSize-1);
}

gettimeofday(&WallEnd,NULL);	//getting the times for when we finish sorting
CPUEnd=clock();
SortWallTime=(float)(WallEnd.tv_sec-WallStart.tv_sec)+((float)(WallEnd.tv_usec-WallStart.tv_usec)/1000000);	//Calculating the elapsed Wall Clock time to sort. Seconds + microseconds/1000000. Done here so the printf isn't a mess

//--------------------------------------------------------------------------------------------------Final Output- Check to see if the array is sorted, if so, output something like shown below, if not print out an error message.										        							       SIZE     THRESHOLD SD PC T CREATE   INIT  SHUFFLE   PART  SRTWall SRT CPU ALLWall ALL CPU	  --------- --------- -- -- - ------ ------- ------- ------- ------- ------- ------- -------	    F:######### ######### ## ## # ##.### ###.### ###.### ###.### ###.### ###.### ###.### ###.###

if(IsSorted(SortMeSize) == true)	//If the array is sorted
{
printf("   SIZE     THRESHOLD SD PC T CREATE   INIT  SHUFFLE   PART  SRTWall SRT CPU ALLWall ALL CPU\n");
printf("  --------- --------- -- -- - ------- ------ ------- ------- ------- ------- ------- -------\n");
	printf("F:%9i %9i ",SortMeSize,Threshold);	
	//Going through and begin printing the inputs
	if(Seed == 1)					
	//If the seed is the default value (effectively none
	{
		printf("00 ");						//print out 00 as seed
	}
	else
	{
		printf("%2i ",Seed);				//else print out seed
	}
	printf("%2i %1i ",Pieces,Maxthreads);	//finish printing the inputs
	printf("%6.3f %7.3f %7.3f %7.3f ",CreateTime,InitializeTime,ShuffleTime,PartitionTime);



	gettimeofday(&TotalEnd,NULL);	//getting the wall clock time we finished at
	TotalWallTime=(float)(TotalEnd.tv_sec-TotalStart.tv_sec)+((float)(TotalEnd.tv_usec-TotalStart.tv_usec)/1000000);	//Calculating the total elapsed wall clock time. Seconds + microseconds/1000000. Done here so the printf isn't a mess

	printf("%7.3f ",SortWallTime);		//print the sorting times (Wall clock then CPU)
	printf("%7.3f ",(float)(CPUEnd-CPUStart)/CLOCKS_PER_SEC);
	printf("%7.3f ",TotalWallTime);		//print the total time take(Wall clock then CPU)
	printf("%7.3f\n",(float)(clock()-TotalTime)/CLOCKS_PER_SEC);
}
else
{
	printf("Error:Array is not sorted.\n");	//else(ie array is not sorted), print this line.
}

return 0;	//End of Program
}
