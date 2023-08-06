// sample_multithread_c_program.c
// compile with: /c
//
//  Bounce - Creates a new thread each time the letter 'a' is typed.
//  Each thread bounces a character of a different color around
//  the screen. All threads are terminated when the letter 'Q' is
//  entered.
//

// #include "include/threads.h"


// // The function getrandom returns a random number between
// // min and max, which must be in integer range.
// #define getrandom( min, max ) (SHORT)((rand() % (int)(((max) + 1) -
//                                (min))) + (min))

// int main(void);                    // Thread 1: main
// void KbdFunc(void);                // Keyboard input, thread dispatch
// void BounceProc(void* pMyID);      // Threads 2 to n: display
// void ClearScreen(void);            // Screen clear
// void ShutDown(void);               // Program shutdown
// void WriteTitle(int ThreadNum);    // Display title bar information

// HANDLE  hConsoleOut;                 // Handle to the console
// HANDLE  hRunMutex;                   // "Keep Running" mutex
// HANDLE  hScreenMutex;                // "Screen update" mutex
// int     ThreadNr = 0;                // Number of threads started
// CONSOLE_SCREEN_BUFFER_INFO csbiInfo; // Console information
// COORD   consoleSize;
// BOOL    bTrails = FALSE;

// HANDLE  hThreads[MAX_THREADS] = { NULL }; // Handles for created threads

// int main(void) // Thread One
// {
//     // Create the mutexes and reset thread count.
//     hScreenMutex = CreateMutexW(NULL, FALSE, NULL);  // Cleared
//     hRunMutex = CreateMutexW(NULL, TRUE, NULL);      // Set

//     // Start waiting for keyboard input to dispatch threads or exit.
//     KbdFunc();

//     // All threads done. Clean up handles.
//     if (hScreenMutex) CloseHandle(hScreenMutex);
//     if (hRunMutex) CloseHandle(hRunMutex);
//     if (hConsoleOut) CloseHandle(hConsoleOut);
// }

// void ShutDown(void) // Shut down threads
// {
//     // Tell all threads to die
//     ReleaseMutex(hRunMutex);

//     while (ThreadNr > 0)
//     {
//         // Wait for each thread to complete
//         WaitForSingleObject(hThreads[--ThreadNr], INFINITE);
//     }
// }

// void KbdFunc(void) // Dispatch and count threads.
// {
//     int         KeyInfo;

//     do
//     {
//         KeyInfo = _getch();
//         if (tolower(KeyInfo) == 'a' &&
//             ThreadNr < MAX_THREADS)
//         {
//             ++ThreadNr;
//             hThreads[ThreadNr] =
//                 (HANDLE)_beginthread(BounceProc, 0, (void*)(uintptr_t)ThreadNr);
//             WriteTitle(ThreadNr);
//         }

//         if (tolower(KeyInfo) == 't')
//         {
//             bTrails = !bTrails;
//         }
//     } while (tolower(KeyInfo) != 'q');

//     ShutDown();
// }

// void BounceProc(void* pMyID)
// {
//     do
//     {
//         // Wait for display to be available, then lock it.
//         WaitForSingleObject(hScreenMutex, INFINITE);

//     }
//     // Repeat while RunMutex is still taken.
//     while (WaitForSingleObject(hRunMutex, 75L) == WAIT_TIMEOUT);
// }