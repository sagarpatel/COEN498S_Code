#include <sys/time.h>
#include <stdio.h>

#define SIZE 256
int main (void)
{



  struct timeval oldTime;
  struct timeval newTime;
  gettimeofday(&oldTime, NULL);
  int usecDT = 0 ;//oldTime.tv_usec;

//  unsigned long long totalUsec = 0;
//  unsigned long long totalMsec = 0;
  unsigned long long totalsec = 0;

  //printf("%d",usecDT);

  int counter = 0;

  gettimeofday(&newTime, NULL);

  while(counter <1000)
  {
    
    newTime.tv_sec += 10000;

    printf("%d\n", newTime.tv_sec);

    counter++;
  }



  return 0;
}
