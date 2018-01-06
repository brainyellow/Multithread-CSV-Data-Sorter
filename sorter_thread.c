#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <getopt.h>
#include <pthread.h>
#include "sorter_thread.h"
#include "mergesort.c"

void *SortFile(void *threadarg)
{
	char *file, *inputdir;
	struct fileThreadData *threaddata;

	threaddata = (fileThreadData *) threadarg;
	file = threaddata->filename;
	inputdir = threaddata->filepath;

	fileNode* newNode;

	//size of CSV
	size_t sizeCSV = 1000;
	char* CSV = malloc(sizeof(char)*sizeCSV);
	
	//stores size of trimBuffer
	size_t sizeTrimBuffer = 50;
	char* trimBuffer = malloc(sizeof(char)*sizeTrimBuffer);
	
	//tokens
	char* tempStr;	
	char* fronttoken;
	char* token;
	char* backtoken;
	char* title;
	
	//size of initial malloc for Movie array
	size_t sizeMovies = 1000;
	//allocate mem for array of movie structs	
	//struct Movie* movies = malloc(sizeof(struct Movie)*(sizeMovies + 50));
	struct Movie* movies = malloc(sizeof(struct Movie)*sizeMovies);

	//compare each token to user input sortby with headerFlag
	int headerFlag = 0;
	
	//counter and variable to keep track of which column to sort by
	int sortColumnCounter = 0;
	int columnToSort = 0;

	//changing working directory for each child process
	chdir(inputdir);
	//creating file stream to read for CSV to be sorted
	FILE* CSVStream;
	CSVStream = fopen(file, "r");
	if (CSVStream == NULL)
	{
		exit(1);
	}
	
	//get first line (headers)
	fgets(CSV, sizeof(char)*sizeCSV,CSVStream);
	
	char* tempHeader = strdup(CSV);
	//making sure the format of the CSV is correct (28 columns)	
	token = strtok(tempHeader, ",");
	while(token != NULL)
	{
		//compare header tokens with user defined sort parameter
		if(strcmp(token, sortby)== 0)
		{
			headerFlag = 1;
			columnToSort = sortColumnCounter;
		}
		sortColumnCounter++;
		token = strtok(NULL, ",");
	}

	//check if what we're sorting has correct headers
	if(headerFlag == 0)
	{
		//stop this process from sorting because it does not contain the correct headers
		exit(1);
	}
	if(sortColumnCounter != 28)
	{
		//stop process if # columns incorrect
		exit(1);
	}

	//resetting counter
	sortColumnCounter = 0;

	token = strtok(CSV, ",");
	while(token != NULL)
	{
		token = strtok(NULL, ",");
		sortColumnCounter++;
	}

	//counter for current struct in movies[]
	int i = 0;	
	//counter for current location in individual struct
	int n = 0;

	while(fgets(CSV, sizeof(char)*sizeCSV, CSVStream))
	{	
		//reallocs when i counter is larger than size of initial movies malloc
		if (i >= sizeMovies)
		{
			//add 1000 movies to size of movies array
			sizeMovies += 1000;		//adding 1000 movies to size of movies array
			//movies = realloc(movies, sizeof(struct Movie)*(sizeMovies + 50));
			movies = realloc(movies, sizeof(struct Movie)*sizeMovies + 1);

			//exception if realloc returns NULL
			if (movies == NULL)
			{
				printf("Realloc failed, not enough memory\n");
				free(movies);
				free(CSV);
				free(trimBuffer);
				free(threadarg);

				pthread_exit(NULL);		
			}
		}

		//dups CSV line
		tempStr = strdup(CSV);
		//stores line until first quote
		fronttoken = strsep(&tempStr, "\"");
		//stores line from first quote > second quote
		title = strsep(&tempStr, "\"");
		
		//stores line from second quote > newline
		backtoken = strsep(&tempStr, "\n");

		/*
		* strdup seg faults if strdup dups a NULL
		* tokenize fronttoken, if not NULL condition for examples where fronttoken is NULL, 
		* such the first element having quotes (robust code)
		*/
		if(fronttoken != NULL)
		{
			tempStr = strdup(fronttoken);
			//reallocing trimBuffer based on size of fronttoken. Since fronttoken is sometimes the whole line, it will be largest possible string to store
			while(strlen(tempStr) > sizeTrimBuffer)		
			{	//doubling size until larger than fronttoken
				sizeTrimBuffer += 50;
				trimBuffer = realloc(trimBuffer, sizeof(char)*sizeTrimBuffer);
				//if not enough mem
				if(trimBuffer == NULL)
				{
					printf("Realloc failed, not enough memory\n");
					free(movies);
					free(CSV);
					free(trimBuffer);
					free(threadarg);

					pthread_exit(NULL);
				}
			}
			/*
			* trimming leading and trailing whitespace for fronttoken because if there is leading whitespace in front of title,
			* fronttoken will have trailing whitespace, which causes extra comma to persist
			*/
			strcpy(trimBuffer, tempStr);									
			strcpy(tempStr, trim(trimBuffer));

			//deleting extra comma to avoid creating extra token
			if (title != NULL)
			{
				tempStr[strlen(tempStr) - 1] = 0;
			}
			
			//tokenize fronttoken using comma delim
			token = strsep(&tempStr, ",");
			while(token != NULL)
			{
				//trimming whitespace in token
				strcpy(trimBuffer, token);
				strcpy(token, trim(trimBuffer));
				//if statements to place current token in correct struct variable
				//Includes all variables
				if(n == 0)
					movies[i].color = token;
				if(n == 1)
					movies[i].director_name = token;
				if(n == 2)
					movies[i].num_critic_for_reviews = token;
				if(n == 3)
					movies[i].duration = token;
				if(n == 4)
					movies[i].director_facebook_likes = token;
				if(n == 5)
					movies[i].actor_3_facebook_likes = token;
				if(n == 6)
					movies[i].actor_2_name = token;
				if(n == 7)
					movies[i].actor_1_facebook_likes = token;
				if(n == 8)
					movies[i].gross = token;
				if(n == 9)
					movies[i].genres = token;
				if(n == 10)
					movies[i].actor_1_name = token;
				if(n == 11)
					movies[i].movie_title = token;
				if(n == 12)
					movies[i].num_voted_users = token;
				if(n == 13)
					movies[i].cast_total_facebook_likes = token;
				if(n == 14)
					movies[i].actor_3_name = token;
				if(n == 15)
					movies[i].facenumber_in_poster = token;
				if(n == 16)
					movies[i].plot_keywords = token;
				if(n == 17)
					movies[i].movie_imdb_link = token;
				if(n == 18)
					movies[i].num_user_for_reviews = token;
				if(n == 19)
					movies[i].language = token;
				if(n == 20)
					movies[i].country = token;
				if(n == 21)
					movies[i].content_rating = token;
				if(n == 22)
					movies[i].budget = token;
				if(n == 23)
					movies[i].title_year = token;
				if(n == 24)
					movies[i].actor_2_facebook_likes = token;
				if(n == 25)
					movies[i].imdb_score = token;
				if(n == 26)
					movies[i].aspect_ratio = token;
				if(n == 27)
					movies[i].movie_facebook_likes = token;
				token = strsep(&tempStr, ",");
				n++;
			}
		}
		//Only tokenize backtoken if NOT NULL
		if(backtoken != NULL)
		{
			tempStr = strdup(backtoken);
			if(title != NULL)
			{
				strcpy(trimBuffer, title);
				strcpy(title, trim(trimBuffer));
				//if statements to place current token in correct struct variable
				//Only including title variable
				if(n == 11)
					movies[i].movie_title = title;
				
				token = strsep(&tempStr,",");	//Get rid of extra comma meant for title containing title
				n++;
			}

			token = strsep(&tempStr, ",");
			while(token != NULL)
			{
				//trimming whitespace
				strcpy(trimBuffer, token);
				strcpy(token, trim(trimBuffer));
				//if statements to place current token in correct struct variable
				//Only includes variables after movie title	
				if(n == 12)
					movies[i].num_voted_users = token;
				if(n == 13)
					movies[i].cast_total_facebook_likes = token;
				if(n == 14)
					movies[i].actor_3_name = token;
				if(n == 15)
					movies[i].facenumber_in_poster = token;
				if(n == 16)
					movies[i].plot_keywords = token;
				if(n == 17)
					movies[i].movie_imdb_link = token;
				if(n == 18)
					movies[i].num_user_for_reviews = token;
				if(n == 19)
					movies[i].language = token;
				if(n == 20)
					movies[i].country = token;
				if(n == 21)
					movies[i].content_rating = token;
				if(n == 22)
					movies[i].budget = token;
				if(n == 23)
					movies[i].title_year = token;
				if(n == 24)
					movies[i].actor_2_facebook_likes = token;
				if(n == 25)
					movies[i].imdb_score = token;
				if(n == 26)
					movies[i].aspect_ratio = token;
				if(n == 27)
					movies[i].movie_facebook_likes = token;
				
				token = strsep(&tempStr, ",");
				n++;
			}
		}
		//resetting n after each line
		n = 0;
		//incrementing i movies
		i++;
	}
	int structCount = i;

	sortbycolumn = columnToSort;
	
	//Sort the movies array using the mergesort function
	mergesort(columnToSort, movies, 0, structCount - 1);

	pthread_mutex_lock(&movielock);

	if(headmovieslist->length == -1)
	{
		headmovieslist->nodesmovies = movies;
		headmovieslist->length = structCount;
		headmovieslist->next = NULL;
	}
	else
	{
		newNode = malloc(sizeof(fileNode));
		newNode->nodesmovies = movies;
		newNode->length = structCount;
		newNode->next = headmovieslist;

		headmovieslist = newNode;
	}
	
	pthread_mutex_unlock(&movielock);

	//freeing mallocs	
	free(CSV);
	free(trimBuffer);
	free(threadarg);
	free(file);
	free(inputdir);

	pthread_exit(NULL);
}


void *SearchDirectory(void *dirpath)
{
	char *inputdir;
	char *newthreaddir;
	pthread_t tid;
	int entrycounter = 0;
	int localtidindex;
	struct dirent *nextentry;
	DIR *dir;
	char *nextdir = calloc(1024, sizeof(char));
	char *tempdir = calloc(1024, sizeof(char));
	char *d_name = calloc(1024, sizeof(char));
	struct fileThreadData *threaddata;

	inputdir = (char *) dirpath;

	if((dir = opendir(inputdir)) == NULL)
	{
		printf("\nCan't open %s\n", inputdir);
		pthread_exit(NULL);
	}

	while((nextentry = readdir(dir)) != NULL)
	{
		//if entry is a csv file then make a new thread for that file
		//if entry is a directory then fork
		//if entry is neither then do nothing and continue

		strcpy(d_name, nextentry->d_name);
		entrycounter++;

		if(entrycounter > 2)
		{
			//check current entry is a directory
			if(nextentry->d_type == DT_DIR)
			{	
				strcpy(tempdir, nextdir);
				strcpy(nextdir, inputdir);
				strcat(nextdir, "/");
				strcat(nextdir, d_name);

				newthreaddir = calloc(1024, sizeof(char));

				//make arg for new thread
				strcpy(newthreaddir, nextdir);

				//make new thread for new directory
				pthread_create(&tid, NULL, *SearchDirectory, (void *) newthreaddir);

				pthread_mutex_lock(&tidlock);
				if(tidindex >= sizeoftids)
				{
					sizeoftids += 10;
					tids = realloc(tids, sizeoftids * sizeof(pthread_t));
					if(tids == NULL)
						return 0;
				}
				localtidindex = tidindex;
				tidindex++;
				pthread_mutex_unlock(&tidlock);

				tids[localtidindex] = tid;

				//pthread_join(tid, NULL);

				strcpy(nextdir, tempdir);
			}
			else
			{
				//check current entry is a csv that needs to be sorted
				char *dot = strrchr(d_name, '.');
				if (dot && !strcmp(dot, ".csv") && (strstr(d_name, "-sorted") == NULL))
				{
					//make arg struct for new thread
					threaddata = (fileThreadData *) malloc(sizeof(fileThreadData));
					threaddata->filename = calloc(1024, sizeof(char));
					threaddata->filepath = calloc(1024, sizeof(char));
					strcpy(threaddata->filename, d_name);
					strcpy(threaddata->filepath, inputdir);

					//make new thread to sort file
					pthread_create(&tid, NULL, *SortFile, (void *) threaddata);

					pthread_mutex_lock(&tidlock);
					if(tidindex >= sizeoftids)
					{
						sizeoftids += 10;
						tids = realloc(tids, sizeoftids * sizeof(pthread_t));
						if(tids == NULL)
							return 0;
					}
					localtidindex = tidindex;
					tidindex++;
					pthread_mutex_unlock(&tidlock);

					tids[localtidindex] = tid;

					//pthread_join(tid, NULL);
				}
			} 
		}
	}
	free(dirpath);
	free(nextdir);
	free(tempdir);
	free(d_name);
	closedir(dir);
	pthread_exit(NULL);
}

void *MergeNodes(void *firstnodetomerge)
{
	//merge the first node and next node
	//so merge firstnode and firstnode->next
	//update first node to be combined and remove firstnode->next

	fileNode *firstnode = (fileNode *) firstnodetomerge;

	if(firstnode->next == NULL)
		pthread_exit(NULL);

	fileNode *secondnode = firstnode->next;

	firstnode->nodesmovies = realloc(firstnode->nodesmovies, sizeof(struct Movie)*(firstnode->length + secondnode->length));

	int i = firstnode->length;
	int j = 0;
	while(j < secondnode->length)
	{
		firstnode->nodesmovies[i] = secondnode->nodesmovies[j];
		i++;
		j++;
	}

	merge(sortbycolumn, firstnode->nodesmovies, 0, firstnode->length, (firstnode->length + secondnode->length - 1));

	pthread_mutex_lock(&movielock);
	if(secondnode->next == NULL)
	{
		firstnode->next = NULL;
		firstnode->length = (firstnode->length + secondnode->length);
	}
	else
	{
		firstnode->next = secondnode->next;
		firstnode->length = (firstnode->length + secondnode->length);
	}
	pthread_mutex_unlock(&movielock);

	free(secondnode->nodesmovies);
	free(secondnode);

	pthread_exit(NULL);
}

int main(int argc, char * argv[])
{
	int option;
	extern int optind;
	extern char *optarg;
	char inputdir[1024];
	char outputdir[1024];
	char tempwd[1024];
	char* tempoutputdir = malloc(1000);
	bool fieldgiven = false;
	bool inputgiven  = false;
	bool outputgiven  = false;
	
	while((option = getopt(argc, argv,"c:d:o:")) != -1)
	{
		switch(option){
			case 'c':
				fieldgiven = true;
				strcpy(sortby, optarg);
				break;
			case 'd':
				inputgiven = true;
				strcpy(inputdir, optarg);
				break;
			case 'o':
				outputgiven = true;
				strcpy(outputdir, optarg);
				break;
			case '?':
				printf("Incorrect flag entered\n");
				return -1;
		}
	}
	
	if(!fieldgiven)
	{
		printf("-c flag not entered\n");
		free(tempoutputdir);
		return -1;
	}

	//input directory is not given and output is given
	if((!inputgiven) && outputgiven)
	{
		getcwd(inputdir, sizeof(inputdir));

		if((outputdir[0] == '.') && (outputdir[1] == '/'))
		{
			getcwd(tempwd, sizeof(tempwd));
			strcpy(tempoutputdir, outputdir);
			//removing dot using strtok
			strcpy(tempoutputdir,strtok(tempoutputdir, "."));
			strcat(tempwd, tempoutputdir);
			strcpy(outputdir, tempwd);
		} 
		else if(outputdir[0] != '/')
		{
			getcwd(tempwd, sizeof(tempwd));
			strcat(tempwd, "/");
			strcat(tempwd, outputdir);
			strcpy(outputdir, tempwd);
		}
	}

	//input directory is given and output is given
	if(inputgiven && outputgiven)
	{
		if((outputdir[0] == '.') && (outputdir[1] == '/'))
		{
			getcwd(tempwd, sizeof(tempwd));
			strcpy(tempoutputdir, outputdir);
			//removing dot using strtok
			strcpy(tempoutputdir,strtok(tempoutputdir, "."));
			strcat(tempwd, tempoutputdir);
			strcpy(outputdir, tempwd);
		} 
		else if(outputdir[0] != '/')
		{
			getcwd(tempwd, sizeof(tempwd));
			strcat(tempwd, "/");
			strcat(tempwd, outputdir);
			strcpy(outputdir, tempwd);
		}
	}

	//input directory is not given and output is not given
	if((!inputgiven) && (!outputgiven))
	{
		//input directory is current directory
		getcwd(inputdir, sizeof(inputdir));
		//output directory is source file directory
	}

	//free pointer
	free(tempoutputdir);
	
	pthread_t tid;
	char *firstthreaddir = calloc(1024, sizeof(char));

	strcpy(firstthreaddir, inputdir);

	sizeoftids = 10;
	tids = malloc(sizeoftids * sizeof(pthread_t));
	if(tids == NULL)
		return 0;
	tidindex = 0;

	headmovieslist = malloc(sizeof(fileNode));
	headmovieslist->length = -1;
	
	pthread_mutex_init(&movielock, NULL);

	pthread_create(&tid, NULL, *SearchDirectory, (void *) firstthreaddir);

	printf("Initial PID: %lu\n", (unsigned long) pthread_self());
	fflush(stdout);
	printf("TIDS of all child threads: ");
	fflush(stdout);

	printf("%lu", (unsigned long) tid);
	fflush(stdout);
	pthread_join(tid, NULL);

	int i = 0;
	while(i < tidindex)
	{
		printf(", %lu", (unsigned long) tids[i]);
		fflush(stdout);
		pthread_join(tids[i], NULL);
		i++;
	}

	int threadcount = tidindex;

	while(headmovieslist->next != NULL)
	{
		tidindex = 0;
		sizeoftids = 10;
		pthread_t *mergetids = malloc(sizeoftids * sizeof(pthread_t));

		pthread_t tid;
		fileNode *myptr;
		fileNode *anotherptr;
		myptr = headmovieslist;

		if(headmovieslist->next->next != NULL)
			myptr = headmovieslist->next->next;

		pthread_create(&tid, NULL, *MergeNodes, (void *) headmovieslist);
		
		//add tid to array
		if(tidindex >= sizeoftids)
		{
			sizeoftids += 10;
			mergetids = realloc(mergetids, sizeoftids * sizeof(pthread_t));
			if(tids == NULL)
				return 0;
		}
		mergetids[tidindex] = tid;
		tidindex++;

		while(myptr->next != NULL)
		{
			anotherptr = myptr->next;
			
			if(myptr->next->next != NULL)
				myptr = myptr->next->next;
			else
				myptr = myptr->next;

			pthread_create(&tid, NULL, *MergeNodes, (void *) anotherptr);
			
			//add tid to array
			if(tidindex >= sizeoftids)
			{
				sizeoftids += 10;
				mergetids = realloc(mergetids, sizeoftids * sizeof(pthread_t));
				if(tids == NULL)
					return 0;
			}
			mergetids[tidindex] = tid;
			tidindex++;
		}
		threadcount += tidindex;

		//wait for tids
		i = 0;
		while(i < tidindex)
		{
			printf(", %lu", (unsigned long) mergetids[i]);
			fflush(stdout);
			pthread_join(mergetids[i], NULL);
			i++;
		}
		free(mergetids);
	}


	if(headmovieslist->length == -1)
	{
		printf("\nTotal number of threads: %d\n", (threadcount + 1));
		return 0;
	}

	printf("\nTotal number of threads: %d\n", (threadcount + 1));

	//Output all movies sorted
	if(!outputgiven)
		strcpy(outputdir,inputdir);
	else if(opendir(outputdir) == NULL)
		mkdir(outputdir, S_IRWXU);
	
	//change dir to output directory
	chdir(outputdir);
	//creating new sorted file name
	char sortedName[1024];
	strcpy(sortedName, "AllFiles-sorted-");
	strcat(sortedName, sortby);
	strcat(sortedName, ".csv");
	FILE* CSVSorted;

	CSVSorted = fopen(sortedName, "w");

	fprintf(CSVSorted, "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes\n");
	
	//Output data by looping through all structs
	//Use new counter to increment through structs
	int k = 0;
	int j = headmovieslist->length;
	while(k < j)
	{
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].color);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].director_name);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].num_critic_for_reviews);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].duration);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].director_facebook_likes);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_3_facebook_likes);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_2_name);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_1_facebook_likes);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].gross);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].genres);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_1_name);
		fprintf(CSVSorted, "\"%s\",", headmovieslist->nodesmovies[k].movie_title);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].num_voted_users);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].cast_total_facebook_likes);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_3_name);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].facenumber_in_poster);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].plot_keywords);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].movie_imdb_link);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].num_user_for_reviews);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].language);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].country);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].content_rating);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].budget);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].title_year);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].actor_2_facebook_likes);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].imdb_score);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].aspect_ratio);
		fprintf(CSVSorted, "%s,", headmovieslist->nodesmovies[k].movie_facebook_likes);
		fprintf(CSVSorted, "\n");
		k++;
	}

	free(headmovieslist->nodesmovies);
	free(headmovieslist);
	free(tids);

	return 0;	
}

//function for trimming whitespace, accepts and returns char*
char* trim(char *str) 
{
    char* end = str + strlen(str) - 1;
    while(*str && isspace(*str)) str++;
    while(end > str && isspace(*end)) *end-- = '\0';
    return str;
}
