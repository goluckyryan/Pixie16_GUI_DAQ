/**********************************************************/
/* Read PXI Set File -- J.M. Allmond (ORNL) -- July 2016  */
/**********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>


#define MOD_NUM 1
#define MOD_SLOT_START 2
#define MOD_LENGTH 1280 //Number of DSP Parameters per Module
#define MOD_TYPE uint32_t //unsigned/signed 2-btye/4-byte; i.e., int16_t, uint16_t, int32_t, uint32_t

int main(int argc, char **argv) {
  
  FILE *fpr;
  MOD_TYPE mod[MOD_LENGTH];
  memset(mod, 0, sizeof(mod));

  int i=0;
  int module=MOD_SLOT_START; 
  
  if ((fpr = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Error, cannot open input file %s\n", argv[1]);
    return 1;
  }


  while (fread(mod, MOD_LENGTH*sizeof(MOD_TYPE), 1, fpr) == 1) {   // read next module settings
          
    //Settings 
    for (i = 0; i < MOD_LENGTH; i++) {

     printf("%d\t%d\t%u\n", module, i, mod[i]);

    } 
        
    module++;    
  }
  
  
  fclose(fpr);
  
  return 0;
}

