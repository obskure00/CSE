Output docs/demo.gif

Set Shell "powershell"
Set FontSize 16
Set Width 780
Set Height 580
Set Padding 18
Set TypingSpeed 60ms

Hide
Type "cd build\bin"
Enter
Sleep 1s
Show

Type ".\emulator.exe kernel.txt bootrom.txt"
Enter
Sleep 2s

Type "ls"
Enter
Sleep 2s

Type "run HELLO.BIN"
Enter
Sleep 2s

Type "run SNAKE.BIN"
Enter
Sleep 1.5s

Type "d"  Sleep 500ms
Type "d"  Sleep 500ms
Type "s"  Sleep 500ms
Type "s"  Sleep 500ms
Type "a"  Sleep 500ms
Type "w"  Sleep 500ms
Sleep 2s