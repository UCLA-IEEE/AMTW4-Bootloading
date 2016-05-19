set tui border-kind ascii
target extended-remote :3333
file main.elf
monitor reset halt
