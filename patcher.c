#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct patch{
    long address;
    char value;
};

struct check{
    long address;
    char value;
};

int main(int argc, char *argv[]){
    FILE *fPatch, *fTarget;
    char szOperation[100], szInstruction[100],szLine[100], szInst2[100];
    char szFilename[100], szOutputFilename[100],szPatchFilename[100], *mem;
    long lFilesize;
    int nPatches = 0, nChecks = 0, bChecksOk = 1;
    struct patch *stPatches;
    struct check *stChecks;

    if(argc == 2){
        strcpy(szPatchFilename,argv[1]);
    } else {
        strcpy(szPatchFilename,"patch.txt");
    }
    fPatch = fopen(szPatchFilename,"r");
    if(!fPatch){
        printf("%s not found\n",szPatchFilename);
        return 1;
    }

    stPatches = (struct patch*)malloc(sizeof(struct patch));
    stChecks = (struct check*)malloc(sizeof(struct check));

    //parsear patch.txt
    while (fscanf(fPatch,"%s",szLine) != EOF){
        if(szLine[0] == '#') continue; // ignorar comentarios
        sscanf(szLine,"%[^(]%s",szOperation,szInstruction);
        
        //Quitar los () de la instruccion
        strcpy(szInst2,szInstruction+1);
        int lastChar = strlen(szInst2)-1;
        szInst2[lastChar] = '\0';
        
        if(!strcmp(szOperation,"File")){
            strcpy(szFilename,szInst2);
        }

        if(!strcmp(szOperation,"Output")){
            strcpy(szOutputFilename,szInst2);
        }
        
        if(!strcmp(szOperation,"Patch")){
            stPatches = realloc(stPatches, (nPatches+1)*sizeof(struct patch));            
            sscanf(szInst2,"%lx%*c%hhx",&stPatches[nPatches].address,&stPatches[nPatches].value);
            nPatches++;
        }
        if(!strcmp(szOperation,"Check")){
            stChecks = realloc(stChecks, (nChecks+1)*sizeof(struct check));
            sscanf(szInst2,"%lx%*c%hhx",&stChecks[nChecks].address,&stChecks[nChecks].value);
            nChecks++;
        }
    }

    printf("Patching file: %s\n",szFilename);

    fTarget = fopen(szFilename,"rb");
    if(!fTarget){
        printf("Can't open target file\n");
        return 1;
    }

    fseek(fTarget, 0, SEEK_END);
    lFilesize = ftell(fTarget);
    rewind(fTarget);

    mem = (char*) malloc(lFilesize);

    fread(mem, sizeof(char), lFilesize, fTarget);
    fclose(fTarget);

    printf("File read successfully, %ld bytes\n", lFilesize);

    printf("Patching\n");

    for(int i = 0; i < nChecks; i++){
        if(mem[stChecks[i].address] != stChecks[i].value){
            bChecksOk = 0;
        }
        //printf("Address: %lx, new value: %x\n",stPatches[i].address,stPatches[i].value);
    }
    
    if(bChecksOk){
        printf("All checks successful\n");
        for(int i = 0; i < nPatches; i++){
            mem[stPatches[i].address] = stPatches[i].value;
            //printf("Address: %lx, new value: %x\n",stPatches[i].address,stPatches[i].value);
        }
        printf("Writing patched output\n");
        fTarget = fopen(szOutputFilename,"wb");
        fwrite(mem,sizeof(char),lFilesize,fTarget);
        fclose(fTarget);
    } else {
        printf("Checks failed, refusing to patch\n");
    }

    free(stPatches);
    fclose(fPatch);
    
    return 0;
}
