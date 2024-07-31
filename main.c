#include "compile.h"
#include "pre_processor.h"

int ic;
int dc;
int has_error;
int cnt_line;
char *file_am_name;

int main(int argc, char* argv[]){
    int i; /* arg index */
	
    char *asfile_name;
    
	/* If no filename received */
	if (argc < 2)
	{
		printf("No input file received.");
		exit(1);
	}
	
	/* Sends all files to compile */
	for (i = 1; i < argc; i++)
	{
		asfile_name = name_file(argv[i],INPUT_FILE);
       
		if(!pre_processor(argv[i])){
                   printf("\nErrors in Macros declarations and implematation, failed to process file '%s.\n",asfile_name);
                    continue;
                }
		compile(argv[i]);
        
                free(asfile_name);
        }
	
	return 0;
}




