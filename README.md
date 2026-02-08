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
     gcc main.c -o main
     ```
2. run the scanner:
   - provide the path to a Unity log file as an argument:
     ```sh
     ./main <path_to_unity_log>
     ```
   - example:
     ```sh
     ./main UnityPlayer.log
     ```
