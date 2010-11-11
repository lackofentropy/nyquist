/* winstuff.c - windows interface routines for xlisp */
/* Written by Chris Tchou. */
/* This file contains the stuff that the other xlisp files call directly. */

/* Changes by Roger Dannenberg, April 2004:
To support interrupts to Lisp processing, XLISP call oscheck frequently to
test for an abort or break condition. This condition can be activated by
special handlers, e.g. if a software interrupt is generated by Ctrl-C.
Alternatively, the application can read ahead and look for break characters
in the input stream. A third approach, implemented by Ning Hu for her
Delphi-based IDE, is to send a Windows message to the process. Unfortunately,
the Java IDE does not support sending a Windows message, nor can console
IO be used to read ahead (console IO does not work because, when started by
Java, Nyquist standard IO is redirected through pipes). The two solutions
to enable break character prcessing seem to be:
  1) extend Java with C code to find the process and send Windows messages
  2) add a thread to perform read ahead and break character processing
Option 2 contains the ugliness to Nyquist IO, which is already big and ugly,
and leaves Java alone, which is something I don't know much about anyway,
so I have chosen option 2: create a thread and read ahead. This uses only
about 50 lines of code.

A shortcoming of this approach is that, except for Ctrl-C, break characters
like ^P, ^B, and ^U require the user to type RETURN to end the input line
and allow the character to be processed.

The thread will set a signal whenever a line of input is delivered so that
Nyquist can block waiting for input. The signal will also be set when a
^C or ^G is detected.

For flexibility, compatibility with the Delphi IDE (NyqIDE) is retained by
continuing to check for Windows process messages.
*/

#include <windows.h>  /* Added by Ning Hu	Apr.2001 */
#include <process.h>  /* Added by Dannenberg Apr 2004 */
#include <signal.h>   /* Added by Dannneberg, Apr 2004 */
#include "exitpa.h"   /* Added by Dannneberg, Apr 2004 */

const char os_pathchar = '\\';
const char os_sepchar = ',';


#undef ERROR
#include <stdio.h>
//#include <QuickDraw.h>	/* for Random */
#include <memory.h>		/* for DisposPtr */
#include <string.h>
//#include <SegLoad.h>	/* for ExitToShell */
#include "xlisp.h"
#include "cext.h"
#include "userio.h"

/* externals */
extern FILE *tfp;  /* transcript file pointer */
extern int cursorPos;
extern char *macgets (void);
//Added by Ning Hu	Apr.2001
extern int _isatty(int);					
extern int redirect_flag;
//Add end

/* local variables */
int lposition;
static char *linebuf = NULL, *lineptr;
static int numChars;

/* input thread */
unsigned long input_thread_handle = 0;
#define input_buffer_max 1024
#define input_buffer_mask (input_buffer_max - 1)
char input_buffer[1024];
volatile int input_buffer_head = 0;
volatile int input_buffer_tail = 0;
HANDLE input_buffer_ready = NULL;

BOOL WINAPI ctrl_c_handler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT) {
        abort_flag = ABORT_LEVEL;
        return TRUE;
    }
    return FALSE;
}


void input_thread_run(void *args)
{
    int c;
    /* this gets called, possible later, in io_init() in userio.c, but
     * that doesn't seem to prevent this thread from being killed by
     * CTRL-C, so call it here to be safe.
     */
    SetConsoleCtrlHandler(ctrl_c_handler, TRUE);

    while (TRUE) {
        int head;
        c = getchar();
        if (redirect_flag == 0 && c == EOF && abort_flag) {
            // you cannot have EOF on a console, but when user types ^C,
            // an EOF is generated for some reason. Ignore it...
            if (abort_flag == ABORT_LEVEL) c = ABORT_CHAR;
            else c = BREAK_CHAR;
        } else if (c == ABORT_CHAR) {
            abort_flag = ABORT_LEVEL;
        } else if (!abort_flag && c == BREAK_CHAR) {
            // notice that a break will be ignored until XLISP
            // handles the ABORT_LEVEL
            abort_flag = BREAK_LEVEL;
        } else if (c == BREAK_CHAR) {
            ; // ignore this because abort_flag is set to ABORT_LEVEL
        } else {
            // insert character into the FIFO
            head = (input_buffer_head + 1) & input_buffer_mask;
            while (head == input_buffer_tail) Sleep(100);
            input_buffer[input_buffer_head] = c;
            input_buffer_head = head;
        }
        if (c == '\n' || abort_flag) {
            SetEvent(input_buffer_ready);
            // wake up Nyquist if it is waiting for input
        }
    } 
    printf("Input thread exiting\n");
}

//int isascii (char c) { return 1; }  /* every char is an ascii char, isn't it? */

void osinit (char *banner) {
//	int i;

    char version[] = "\nWindows console interface by Chris Tchou and Morgan Green.\n";
//	InitMac ();  /* initialize the mac interface routines */
//	lposition = 0;  /* initialize the line editor */
//	for (i = 0; banner[i] != '\0'; i++) macputc (banner[i]);
//	for (i = 0; version[i] != '\0'; i++) macputc (version[i]);
    printf(banner);
    printf(version);

    // Added by Ning Hu, to communicate between compiler GUI and Nyquist console	Apr.2001
    if (_isatty( _fileno( stdout ) ) ){
        redirect_flag = 0;
#ifdef DEBUG
        printf( "stdout has not been redirected to a file\n" );		//for debugging use
#endif
        /* if this is a console, disable ^C 
            (this doesn't seem to retain other features like input editing)

        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                       ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT); 
         */
    } else {
        redirect_flag = 1;
#ifdef DEBUG
        printf( "stdout has been redirected to a file\n");			//for debugging use
#endif
    }
    // Add End

    /* SetConsoleCtrlHandler(NULL, TRUE); */
    // signal when input is ready
    input_buffer_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (input_buffer_ready == NULL) {
        printf("Unable to create Event object\n");
        EXIT(1);
    }
    // create thread to process input
    /**/
    input_thread_handle = _beginthread(input_thread_run, 0, NULL);
    if (input_thread_handle == -1) {
        printf("Unable to create input thread, errno = %d\n", errno);
        EXIT(1);
    }
    /**/
}

/* osrand - return next random number in sequence */
long osrand (long rseed) {
#ifdef OLDBUTINTERESTING
// note that this takes a seed and returns a big number,
// whereas I think XLisp's RANDOM is defined differently
    long k1;

    /* make sure we don't get stuck at zero */
    if (rseed == 0L) rseed = 1L;

    /* algorithm taken from Dr. Dobbs Journal, November 1985, page 91 */
    k1 = rseed / 127773L;
    if ((rseed = 16807L * (rseed - k1 * 127773L) - k1 * 2836L) < 0L)
    rseed += 2147483647L;

    /* return a random number between 0 and MAXFIX */
    return rseed;
#endif
    return rand() % rseed;	// rseed is a misnomer
}

FILE *osaopen (char *name, char *mode) {
    return fopen (name, mode);
}

FILE *osbopen (char *name, char *mode) {
    char nmode[4];
    strcpy (nmode, mode); strcat (nmode, "b");
    return (fopen (name, nmode));
}

int osclose (FILE *fp) { return (fclose (fp)); }
int osaputc (int ch, FILE *fp) { return (putc (ch, fp)); }
int osbputc (int ch, FILE *fp) { return (putc (ch, fp)); }

/* osagetc - get a character from an ascii file */
int osagetc(fp)
  FILE *fp;
{
    return (getc(fp));
}

extern int abort_flag;
extern int redirect_flag;			//Added by Ning Hu	Apr.2001

int ostgetc (void) {
    //  return getchar(); //Old code--removed by Ning Hu
    /* Added by Ning Hu		Apr.2001 
    int ch = getchar();
    oscheck(); // in case user typed ^C
    if (ch == BREAK_CHAR && abort_flag == BREAK_LEVEL) {
        abort_flag = 0;
    }
    switch (ch) {
          case ABORT_CHAR:	// control-c
            xltoplevel();
            break;
          case '\025':	// control-u
            xlcleanup();
          case '\020':	// control-p
            xlcontinue();
            break;
          case BREAK_CHAR:
            ostputc('\n');	// control-b
            xlbreak("BREAK",s_unbound);
            break;
          default:
            break;
    }
    return ch;
    //Add End
    */
    int c;
    while (input_buffer_tail == input_buffer_head) {
        oscheck();
        WaitForSingleObject(input_buffer_ready, INFINITE);
    }
    c = input_buffer[input_buffer_tail];
    input_buffer_tail = (input_buffer_tail + 1) & input_buffer_mask;
    if (c == '\025') { // control-u
        xlcleanup();
    } else if (c == '\020') {
        xlcontinue();
    }
    return c;
}


void ostputc (int ch) {
//	macputc (ch);
    putchar(ch);			// console

    if (tfp) osaputc (ch, tfp);
}


void osflush (void) {
    lineptr = linebuf;
    numChars = 0;
    lposition = 0;
}


void oscheck (void) {				
    MSG lpMsg;
    // check_aborted();	-- call to userio.c superceded by code here in winstuff.c
//	printf("Current Thread: %d\n", GetCurrentThreadId());		//for debugging use
    if ((redirect_flag) && (PeekMessage(&lpMsg, NULL, 0, 0, PM_REMOVE)!=0)) { 
        if (lpMsg.message == WM_CHAR) {
            switch (lpMsg.wParam) {
                case ABORT_CHAR: abort_flag = ABORT_LEVEL;
                                break;
                case BREAK_CHAR: // for nyquist, defined to be 2
                case 7:          // NyqIDE sends 7 (BEL) as break character
                    abort_flag = BREAK_LEVEL;
                    break;
            }
            // printf("Get message: %d %d %d\n", lpMsg.wParam, BREAK_CHAR, abort_flag);				//for debugging use
        }
    }	
    if (abort_flag == ABORT_LEVEL) {	
        abort_flag = 0;
        osflush();
        xltoplevel();
    } else if (abort_flag == BREAK_LEVEL) {
        abort_flag = 0;
        osflush();
        xlbreak("BREAK", s_unbound);
    }
}
//Update end

void oserror (char *msg) {
    char line[100], *p;
    sprintf (line,"error: %s\n",msg);
    for (p = line; *p != '\0'; ++p) ostputc (*p);
}

void osfinish(void) {
    portaudio_exit();
    /* dispose of everything... */
//	if (linebuf) DisposPtr (linebuf);
//	MacWrapUp ();
//	ExitToShell ();
}

int renamebackup (char *filename) { return 0; }


long randomseed = 1L;

long random () {
// note that this takes a seed and returns a big number,
// whereas I think XLisp's RANDOM is defined differently
    long k1;

    /* algorithm taken from Dr. Dobbs Journal, November 1985, page 91 */
    k1 = randomseed / 127773L;
    if ((randomseed = 16807L * (randomseed - k1 * 127773L) - k1 * 2836L) < 0L)
      randomseed += 2147483647L;

    /* return a random number between 0 and MAXFIX */
    return randomseed;
}

/* Added by Ning Hu		May.2001 
xsetdir - set current directory of the process */
LVAL xsetdir() {
    TCHAR ssCurDir[MAX_PATH], szCurDir[MAX_PATH];

    strcpy(ssCurDir, getstring(xlgastring()));
    xllastarg();
    if (SetCurrentDirectory(ssCurDir)) {
        if (GetCurrentDirectory(
            sizeof(szCurDir)/sizeof(TCHAR), szCurDir)) {	
        /* create the result string */
            stdputstr("Current Directory: ");
            stdputstr(szCurDir);
            stdputstr("\n");
        }	
        else stdputstr("Directory Setting Error\n");
    }
    else stdputstr("Directory Setting Error\n");

    /* return the new string */
    return (NIL);
}
//Updated End
