Coding guidelines:
==================
Follow good coding standards. Google's C++ style guide is a good
reference for this.

http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml


Additional notes and minor deviations from Google's coding guidelines
=====================================================================
1. Every name in the program (be it class name, function name
   or variable name) should be meaningful, but avoid very long
   names. Give some thought to design decisions - where to 
   make a class, a method etc.

2. Source code must be indented with a tabspace of 4. Do not use 
   the tab character (\t) for indenting, use 4 spaces (in vim/gvim, 
   this can be done by the command "set expandtab").

3. Each source line must be limited 72 columns, break a statement
   at appropriate places.

4. Write a Makefile to build the source code. Read about "make"
   and Makefile.

