# Shell - Ananya Balaji

## To run Interactive Mode, first `make`, then perform `./mysh`
## To run Batch Mode, run `./mysh _____`, where ____ is the file name
    - to exit the Batch Mode in error situation like Interactive Mode perform Control + C

## Certain Test Cases
### Plain File with All Inputs and Arguments

## Things to Note
- When using grep as such `grep "bar" bar.txt`, use as `grep bar bar.txt` instead. In my implementation quotations are not considered in the execv implementation.
- `apple.txt` and `apple` files are considered different in my shell

## Part 1: Wildcard (Wildcard) Test Cases
1. **Basic Wildcard Matching**
   - Example: `ls foo*bar`
     - Lists all the files in the current directory starting with foo and ending with bar.

2. **Wildcard in Path**
   - Example: `ls /path/to/baz/foo`

## Part 2: Redirection (Redirection) Test Cases
1. **Input Redirection**
   - Example 1: `cat < input.txt`
     - Reads contents of input.txt and prints to the console.
   - Example 2: `grep i < input.txt`
     - Searches for the keyword 'i' in input.txt and prints matching lines.

2. **Output Redirection**
   - Example 1: `echo "hi!" > output.txt`
     - Writes "hi!" to the file output.txt.
   - Example 2: `ls -l > file_list.txt`
     - Lists files in the current directory and saves the output to file_list.txt.
   - Example 3: `which ls > whichoutput.txt`
     - Locate files with associate command ls and redirects it to whichoutput.txt. 

3. **Combining Input & Output Redirection**
   - Example: `sort < unsorted.txt > sorted.txt`
     - Reads lines from unsorted.txt and sorts them alphanumerically in sorted.txt.
   - **Bonus Example**: `./my_program < input_data.txt > output_data.txt`
     - Runs a custom program (my_program) with input from input_data.txt and writes the output to output_data.txt.

4. **Error Detection Redirection**
   - Example: `cat < validFileNoRead.txt`
     - Tries to print the output of validFileNoRead.txt, which has no read permissions. Returns permission denied.
   - Example: `cat >>`
     - Double Redirection (>>) - my program does not implement APPEND (shell reports this as error)


## Part 3: Pipelines (Pipes) Test Cases
1. **General Pipes Cases**
   - Example 1: `ls -l | grep ".txt"`
     - Prints all files with .txt at the end. Limited to two (2) processes in the pipeline.
   - Example 2: `cat filterationLines.txt | grep "pattern"`
     - Uses a pipeline to filter lines containing the word "pattern" from the contents of input.txt.
   - Example 3: `echo "output redirection file pipes" | tee output.txt`
     - Writes the above string to a pipe and uses tee to save the response to output.txt.
   - Example 4: `cat file1.txt | sort`
     - Reads the contents of file1.txt and sorts the lines (printing the output).

2. **Pipes and Redirection**
   - Example 1: `cat filterationLines.txt | grep "pattern" > results.txt`
     - Uses a pipeline to filter lines containing the word "pattern" from file1.txt and saves results to results.txt.
   - Example 2: `cat repeats.txt | uniq > uniques.txt`
     - Prints out the results of repeats.txt, removes duplicated consecutive lines, and puts them in uniques.txt.

## Part 4: Syntax Errors Test Cases 
   - Example 1: `cat | `, `grep | `,  `echo | `, '| cat', '* |', ' < | *'
     - Prints out that it is an invalid pipe command
   - Example 2) `cat >`, `ls <`, `cat < >`, '<> cat', '< cat'
     - Exits the shell and says the syntax error with the token you mentioned
     - '> cat' creates an empty file (not visible in directory) similar action as real shell
   - Example 3) `cd` with zero or 2+ arguments
     - Prints an error if there is 0 argument or too many involved
   - Example 4) `pwd apple`, `pwd a b c`
     - Ignores any arguments (says so) and performs pwd as regular
   - Example 5) `which`, `which cd`, `which cd ls`
     - Says missing argument if doesn't exist used
     - 1 or more arguments: prints all of the bins they may be in
     - Lets say did those not used and used arguments 'which cd apple ls'
       - Then prints all locations that exist before the non-existing command `apple`, which is an error and therefore exits the shell before further processing
   - Example 6) 'apple cd', 'apple which ls', '* cat'
     - execv: detection of no file or directory

## Part 5: Conditional Execution (Then, Else)
   - 'cd' always returns status of 0 even if the file/directory does not exist
     - Therefore 'then' and 'else' will proceed as if the prior 'cd' command was successful
   - `cat wc1.txt; then cat appleyanu; else echo "hi"` [THEN, ELSE]
     - Status of 0, 1, then else can execute echo "hi"
   - `cat apple, else echo "correct", then echo "continued"` [ELSE, THEN]
     - Status of 1, else executes with status 0, then executes because of 0
   - `cat apple, else echo "correct", else echo "incorrect"` [ELSE, ELSE]
     - Status of 1, else executes with status 0, then else for command 0 not execute (so nothing done)
   - `cat wc1.txt, then echo "correct", then echo "yippee"` [THEN, THEN]
     - Status of 0, then executes with status 0, the new then executes with status 0

   - cat then | grep else (works)
   - cat then | else grep "pattern" ()

## Part 6: Equivalent Commands (STDIN, STDOUT)
   -  similar to the project document has mentioned the following `foo`, `quux`, `bar`, `baz` in the following order, I have implemented it with real arguments/commands
   - #foo quux < bar > baz
     - `grep bar < bar.txt > baz.txt`
   - #foo < bar quux > baz
     - `grep < bar.txt bar > baz.txt`
   - #foo > baz < bar quux
     - `grep > baz.txt < bar.txt bar`

## Part 7: Using ln -s (to )
   - `ln -s /usr/bin/grep foo`
   - `cp bar.txt bar`
   - `vi bar` ---> store bar and quux words inside
   - `foo quux < bar > baz`
     - Output: prints quux (CORRECT)

## Part 6: Project Description Example
   - Example: `then foo < bar baz`
     - My version of this was performing `then wc < gef.txt input.txt`
       - Output: 3  3  6   input.txt
       - Likened to `wc input.txt < gef.txt`

## External Stuff Noticed (Shell Performs the Same; ):
## NOTE: Can only exit these commands by performing Control + C (CMD + C on Mac)
   - Example 1: 'cat'
     - Performs the same thing like the shell in terms of repeating what is printed out, exit command does not work as well
   - EXAMPLE 2: 'grep'
     - Performs the same thing like the shell in terms of not printing out any new prompt and adding any random words
   - EXAMPLE 3: 'sort | uniq > uniques.txt'
     - Performs the same thing like the shell in terms of not printing out stuff
