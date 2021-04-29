# MiniShell
#### The specifications for the shell
* Prompts "mini-shell>" in front of each command that is typed out.
* The maximum input size(i.e. BUFFER SIZE) on a given line will be 80 characters for the shell.
* When the user presses Ctrl+C the text 'mini-shell terminated' is printed out.
* The child processes should run in the foreground by default until it is completed.
* If the user adds the pipe '|' then the output of one process should be the input to the next process.
* Assumes only one '|' will be used at most. e.g. ls -l | wc
* The shell should has some built-in functions. These include `exit` to terminate the shell, `cd` so one may change directories, and `help` explaining all the built-in commands available in your shell.
* If ones launches another shell from within the previous shell, exit only closes out of the most recently launched shell.
* If a command is not found, your shell should print out an error message, and resume execution. e.g. mini-shell>Command not found--Did you mean something else?
* Shell command 'guessinggame' plays a little game in the terminal for guessing random numbers.
