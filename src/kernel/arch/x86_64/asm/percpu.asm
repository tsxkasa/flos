; extern char _percpu_start[];
extern _percpu_start
; extern char _percpu_end[];
extern _percpu_end

; puts the loaded offset in specified register (given by %1)
; TODO: find a cleaner way to do this at compile/link time
%macro PERCPU_OFFSET 2
  lea %1, [rel %2]
  lea r11, [rel _percpu_start]
  sub %1, r11
%endmacro
