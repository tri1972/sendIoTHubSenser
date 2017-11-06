#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <syslog.h>

static int daemonized = 0; /* daemonized flag */

int daemonize(
	      const char* pidfilepath,
	      const char *syslog_ident,
	      int syslog_option,
	      int syslog_facility)
{
  printf("daemonize init start\n");
  daemonized = 0; /* initialize */
  if (daemon(1, 0) != -1) {
    //daemonの第一引数については０以外を設定しないとrootディレクトリにワーキングディレクトリが変更されてしまう
    //そうすると書き込めないためこの後のdaemonプロセスでファイルの書き込みを行うと失敗するため注意!!
    /* success to daemonize. */
    daemonized = 1;

    /* open syslog */
    /*
      if(syslog_ident) {
      openlog(syslog_ident, syslog_option, syslog_facility);
      }
    */

    /* write daemon pid to the file */
    if(pidfilepath) {
      FILE* pidfile = fopen(pidfilepath, "w+");
      if(pidfile!=NULL){
	if (pidfile) {
	  int pid = getpid();
	  fprintf(pidfile, "%d\n", pid);
	} else {
	  syslog(LOG_ERR,"daemonize() : failed to write pid.");
	}
	fclose(pidfile);
      }else{
	syslog(LOG_ERR,"daemonize() : pidfile cant open.");
      }
    }
  }
  return daemonized;
}
