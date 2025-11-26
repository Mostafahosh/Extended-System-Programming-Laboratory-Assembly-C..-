#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
    for(int i = 0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
    }
  return mapped_array;
}
 
char dprt(char c) {
    printf("%d\n", c);
    return c;
}

char my_get(char c) {
    return fgetc(stdin);
}

char cxprt(char c) {
    if (c >= 0x20 && c <= 0x7E)
        printf("%c ", c);
    else
        printf(". ");
    printf("%02x\n", (unsigned char)c);
    return c;
}

char encrypt(char c) {
    if (c >= 0x1F && c <= 0x7E)
        return c + 1;
    return c;
}

char decrypt(char c) {
    if (c >= 0x21 && c <= 0x7F)
        return c - 1;
    return c;
}

struct fun_desc {
  char *name;
  char (*fun)(char);
  };

int main(int argc, char **argv){
    struct fun_desc menu[] = { {"Get String",my_get}, {"Print Decimal" ,dprt},{"Print Char and Hex",cxprt },{"Encrypt" ,encrypt},{"Decrypt" ,decrypt},{ NULL, NULL } };
    char* carray = calloc(5,sizeof(char));
    int bound = (sizeof(menu) / sizeof(struct fun_desc)) -1 ;
    char line[256]; 

    while (1) {
        printf("Select operation from the following menu:\n");
        for (int i = 0; i < bound; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option: ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            free(carray);
            break;
        }

        int number;
        sscanf(line, "%d\n", &number);

        if (number < 0 || number >= bound){
            printf("\nNot within bounds\n");
            break;
        }
        printf("\nWithin bounds\n");
        char *new_array = map(carray, 5, menu[number].fun);
        free(carray);
        carray = new_array; 
    }
    free(carray);
    return 0;
}