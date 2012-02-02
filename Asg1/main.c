#include <sys/time.h>
#include <stdio.h>

#define SIZE 256

int main (void)
{



  struct timeval oldTime;
  struct timeval newTime;
  gettimeofday(&oldTime, NULL);
  int usecDT = 0 ;//oldTime.tv_usec;

  unsigned long long totalUsec = 0;
  unsigned long long totalMsec = 0;
  unsigned long long totalsec = 0;

  //printf("%d",usecDT);

  int counter = 0;

  while(counter <1300)
  {
    

    //get current time
    gettimeofday(&newTime, NULL);

    usecDT = newTime.tv_usec - oldTime.tv_usec;

    totalUsec = (unsigned long long)usecDT;
    totalMsec += totalUsec/1000;
    totalsec  = totalMsec/1000;

    gettimeofday(&oldTime, NULL);


    printf("\n\n %d",totalUsec);
    printf("\n %d", totalMsec);
    printf("\n %d", totalsec);

    counter++;
  }





  return 0;
}
