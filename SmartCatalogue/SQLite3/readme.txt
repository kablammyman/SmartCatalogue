I changewd the names of the folders to src and bin for easy use.


1) use the vis studio cmd prompt to get to it, type in "dev" in the search

////////////////////////Building A Windows DLL/////////////////////////////////////

To build a DLL of SQLite for use in Windows, first acquire the appropriate amalgamated source code files, sqlite3.c and sqlite3.h. 
These can either be downloaded from the SQLite website or custom generated from sources as shown above.

With source code files in the working directory, a DLL can be generated using MSVC with the following command:

when making the dll
cl sqlite3.c -link -dll -out:sqlite3.dll

when making the lib: (make sure you have the def file in the src so you dont have to change dirs)
LIB /DEF:sqlite3.def /OUT:sqlite3.lib


The above command should be run from the MSVC Native Tools Command Prompt. If you have MSVC installed on your machine, you probably have multiple versions of this Command Prompt, 
for native builds for x86 and x64, and possibly also for cross-compiling to ARM. Use the appropriate Command Prompt depending on the desired DLL.






here is the info i got from some random blog:

Using SQLite in a Native Visual C++ Application
Posted on March 21, 2011 by dcravey
As promised, here is the Native Visual C++ followup to my previous blog post.  I hope you find this helpful!

Create a new C++ Win32 Console application.
Download the native SQLite DLL from: http://sqlite.org/sqlite-dll-win32-x86-3070400.zip
Unzip the DLL and DEF files and place the contents in your project’s source folder (an easy way to find this is to right click on the tab and click the “Open Containing Folder” menu item.
Open a “Visual Studio Command Prompt (2010)” and navigate to your source folder.
Create an import library using the following command line: LIB /DEF:sqlite3.def
Add the library (i.e. sqlite3.lib) to your Project Properties -> Configuration Properties -> Linker -> Additional Dependencies.
Download http://sqlite.org/sqlite-amalgamation-3070400.zip
Unzip the sqlite3.h header file and place into your source directory.
Include the the sqlite3.h header file in your source code.
You will need to include the sqlite3.dll in the same directory as your program (or in a System Folder).
Alternatively you can add the sqlite3.c file in your project,  and the SQLite database engine will be included in your exe.  After you add sqlite3.c to your project (right click the project and choose to add an existing file), you will need to right click on the sqlite3.c file in the Solution Explorer and change: Properties->Configuration Properties->C/C++->Precompiled Headers->Precompiled Header = Not Using Precompiled Headers
Here is some example code to get you started:

#include "stdafx.h"
#include <ios>
#include <iostream>
#include "sqlite3.h"
 
using namespace std;
 
int _tmain(int argc, _TCHAR* argv[])
{
   int rc;
   char *error;
 
   // Open Database
   cout << "Opening MyDb.db ..." << endl;
   sqlite3 *db;
   rc = sqlite3_open("MyDb.db", &db);
   if (rc)
   {
      cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl << endl;
      sqlite3_close(db);
      return 1;
   }
   else
   {
      cout << "Opened MyDb.db." << endl << endl;
   }
 
   // Execute SQL
   cout << "Creating MyTable ..." << endl;
   const char *sqlCreateTable = "CREATE TABLE MyTable (id INTEGER PRIMARY KEY, value STRING);";
   rc = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
   if (rc)
   {
      cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
      sqlite3_free(error);
   }
   else
   {
      cout << "Created MyTable." << endl << endl;
   }
 
   // Execute SQL
   cout << "Inserting a value into MyTable ..." << endl;
   const char *sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
   rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
   if (rc)
   {
      cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
      sqlite3_free(error);
   }
   else
   {
      cout << "Inserted a value into MyTable." << endl << endl;
   }
 
   // Display MyTable
   cout << "Retrieving values in MyTable ..." << endl;
   const char *sqlSelect = "SELECT * FROM MyTable;";
   char **results = NULL;
   int rows, columns;
   sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
   if (rc)
   {
      cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
      sqlite3_free(error);
   }
   else
   {
      // Display Table
      for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
      {
         for (int colCtr = 0; colCtr < columns; ++colCtr)
         {
            // Determine Cell Position
            int cellPosition = (rowCtr * columns) + colCtr;
 
            // Display Cell Value
            cout.width(12);
            cout.setf(ios::left);
            cout << results[cellPosition] << " ";
         }
 
         // End Line
         cout << endl;
 
         // Display Separator For Header
         if (0 == rowCtr)
         {
            for (int colCtr = 0; colCtr < columns; ++colCtr)
            {
               cout.width(12);
               cout.setf(ios::left);
               cout << "~~~~~~~~~~~~ ";
            }
            cout << endl;
         }
      }
   }
   sqlite3_free_table(results);
 
   // Close Database
   cout << "Closing MyDb.db ..." << endl;
   sqlite3_close(db);
   cout << "Closed MyDb.db" << endl << endl;
 
   // Wait For User To Close Program
   cout << "Please press any key to exit the program ..." << endl;
   cin.get();
 
   return 0;
}
