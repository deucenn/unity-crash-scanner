Script to scan Unity log files for crash information

#### Features

- parses Unity log files for crash reports
- extracts stack traces and error messages
- outputs summary of crash events

#### Usage

1. build the program:
   - open a terminal in the project directory.
   - run:
     ```sh
     gcc main.c -o crash_scanner -lshlwapi -lshell32
     ```
2. run the scanner:
   - example:
     ```sh
     ./crash_scanner 
     ```
3. open the game

4. after you closed your game or it crashed you can see a more detailed overview in the program window
