#include <time.h>
#include <stdio.h>

#define SIZE 256



int main (void)
{
  /*
  time_t now;
  struct tm ts;
  char buf[80];

  //Get current time
  time(&now);

  //Format time
  ts = *localtime(&now);
  strftime(buf, sizeof(buf),"%a %Y-%m-%d %H:%M:%S %Z", &ts);
  printf ("%s\n", buf);
*/

/*
  struct tm t;
  time_t t_of_day;
  t.tm_year = 2011-1900;
  t.tm_mon = 7;           // Month, 0 - jan
  t.tm_mday = 8;          // Day of the month
  t.tm_hour = 16;     
  t.tm_min = 11;
  t.tm_sec = 42;
  t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
  t_of_day = mktime(&t);
  printf("seconds since the Epoch: %ld\n", (long) t_of_day);

*/


  printf("\n Enter time and date in following format : YYYY MM DD HOUR MIN SEC \n");

  struct tm userTimeInput;
  time_t secondFromTime;

  int year;
  int month;

  char buf[100];
  fgets(buf, sizeof(buf), stdin);

  int actualYear;

  sscanf(buf, "%d %d %d %d %d %d", &actualYear, 
                                &userTimeInput.tm_mon,
                                &userTimeInput.tm_mday,
                                &userTimeInput.tm_hour,
                                &userTimeInput.tm_min,
                                &userTimeInput.tm_sec);

  userTimeInput.tm_year = actualYear - 1900;



  userTimeInput.tm_isdst = -1; 

  secondFromTime = mktime(&userTimeInput);
  printf("seconds between epoch set time: %ld\n", secondFromTime);

  unsigned long uL = (unsigned long)secondFromTime;
  float f = (float)uL;
  printf("seconds between epoch set time: %ld\n", uL);
  printf("seconds between epoch set time: %f\n", f);

  return 0;
}
