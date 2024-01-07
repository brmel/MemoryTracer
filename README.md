# MemoryTracerLib

MemoryTracerLib is a C++ static library designed to assist you in monitoring the Virtual Memory of a specific process over time. It allows you to display data on the screen, export well-formatted data to a file, and provides control over the snapshot frequency.
You can integrate it into your code, utilizing it as a callback function to precisely control when to take a snapshot. Alternatively, you can employ it in another process if you prefer not to modify the code of the process you wish to track.
This project is inspired from https://james-ross.co.uk/projects/vmmap

Example : 
```
#include "snapshotmngr.h"
int _tmain(int argc, _TCHAR* argv[])
   {
   Z_UINT32 ProcessPid = 30004;
   CSnapshotMngr MyMemTracer(ProcessPid);
   MyMemTracer.PrintNow();
   return 0;
   }
```

If you have any feedback about the application, you can submit a new issue here : https://github.com/brmel/MemoryTracerLib/issues
