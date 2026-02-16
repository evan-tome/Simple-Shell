Simple Shell User Manual
---------------------------------------------------
Author: Evan Tome

Date: 02/13/2026

Overview
--------
This is a simple UNIX-like shell program called `myshell`. It allows users to run both internal and external commands from a command-line interface. The shell supports basic commands, input/output redirection, and background execution. This manual explains how to use the shell and its key features.

1. Description of Operation and Commands
---------------------------------------------------
After launching `myshell`, the prompt displays the current directory:

    >myshell:/current/directory$

You can type commands at this prompt. Commands can be **internal** (built into the shell)
or **external** (system programs).  

**Internal commands**:
- `quit`      : Exit the shell.
- `cd <dir>`  : Change the current working directory to `<dir>`. Without arguments, prints current directory.
- `clr`       : Clear the screen.
- `dir <dir>` : List contents of `<dir>`. If no directory is given, lists current directory.
- `environ`   : Print all environment variables.
- `echo <text>` : Display `<text>` on the screen.
- `help`      : Display this help text.
- `pause`     : Waits for the user to press Enter before continuing.

**External commands**:  
Any command available in your system PATH can be executed, e.g., `ls`, `pwd`, `grep`.

2. Description of Environment Concepts
--------------------------------------------------
The shell uses environment variables to store information about the session.

- `shell` : The full path of the running shell program.
- `parent`: Set in child processes, contains the path of the parent shell.
- `PWD`   : The current working directory, updated automatically when `cd` is used.

You can access environment variables using `environ`

3. Description of I/O Redirection
--------------------------------------------
I/O redirection allows the user to read input from a file or write output to a file.
The shell supports:

- Output redirection:  
  `>`  : Write output to a file (overwrite existing content).  
  `>>` : Write output to a file (append if file exists).

  Example:  
      echo Hello > file.txt
      dir >> listing.txt

If the specified file does not exist, it will be created  

- Input redirection (only for external commands):  
  `<` : Use a file as input instead of the keyboard.  

  Example:  
      sort < unsorted.txt

**Notes**:  
- Internal commands (`dir`, `environ`, `echo`, `help`) support output redirection.  
- Input redirection is not supported for internal commands.

4. Description of Background Execution
--------------------------------------------------
Background execution allows a program to run without blocking the shell prompt.  
- Use `&` at the end of an external command to run it in the background.  
- The shell will immediately return to the prompt while the program runs.  

Example:  
    sleep 10 &  
    >myshell:/current/directory$   (prompt is immediately available)

Internal commands **do not support** background execution; the `&` symbol is ignored.

5. Using Batch Files as Input
-----------------------------------------------
- Batch files can be executed by passing the filename as an argument:  

      ./myshell batchfile.txt

End of Manual
-------------
