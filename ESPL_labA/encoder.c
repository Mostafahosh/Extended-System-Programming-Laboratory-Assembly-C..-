#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int debugMode = 1;
unsigned char password[] = "my_password1";
char *key = "0";  
int add = 1; 

FILE *infile = NULL;
FILE *outfile = NULL;

char encode(char c,int digit) {
     if (c >= 'A' && c <= 'Z') {
        if (add)
            return 'A' + (c - 'A' + digit) % 26;
        else
            return 'A' + (c - 'A' - digit + 26) % 26;
    } else if (c >= '0' && c <= '9') {
        if (add)
            return '0' + (c - '0' + digit) % 10;
        else
            return '0' + (c - '0' - digit + 10) % 10;
    }
    return c;
}

int main(int argc, char **argv) {
    infile=stdin;
    outfile=stdout;


    for (int i = 1; i < argc; i++){
        if(debugMode){
            fprintf(stderr,argv[i]);
            fprintf(stderr,"\n");
        }
        if(strcmp(argv[i], "-D") == 0){
            debugMode = 0;
        }
        else if(strncmp(argv[i],"+D",2)==0){
            if(strcmp(argv[i]+2,(char *)password)==0){
                debugMode = 1;
            }
            else{
                fprintf(stderr, "Wrong password!\n");
            }
        }
        
        if (argv[i][0] == '+' && argv[i][1] == 'E') {
            key = argv[i] + 2;
            add = 1;
        } else if (argv[i][0] == '-' && argv[i][1] == 'E') {
            key = argv[i] + 2;
            add = 0;
        }
        if(argv[i][0]=='-' && argv[i][1]=='i'){
            infile = fopen(argv[i]+2,"r");
            if(infile==NULL){
                fprintf(stderr, "Error opening input file!\n");
                return 1;
            }
        }
        else if(argv[i][0]=='-' && argv[i][1]=='o'){
            outfile = fopen(argv[i]+2,"w");
            if(outfile==NULL){
                fprintf(stderr, "Error opening output file!\n");
                return 1;
            }
        }
        
    }

    int c;
    int i = 0;
    int key_len = strlen(key);

    while ((c = fgetc(infile)) != EOF) {
        int digit = key[i] - '0';
        c = encode((char)c, digit);
        fputc(c, outfile);
        i = (i + 1) % key_len;
    }
    if (infile != stdin)
        fclose(infile);
    if (outfile != stdout)
        fclose(outfile);

    return 0;
    
}