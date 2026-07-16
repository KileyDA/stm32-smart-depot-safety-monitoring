## LED Debugging Note: `lvalue required as left operand of assignment`

While testing the LED register-level code, I encountered the following build error:

```text
../Src/main.c:50:25: error: lvalue required as left operand of assignment
   50 |         AHB1_EN_R       |= GPIOAEN;
      |                         ^~
```

This error usually means that the value on the left side of an assignment operation is not writable. In bare-metal register programming, this can happen if a register macro is defined as an address only, instead of being defined as a dereferenced pointer to that address.

For example, the register macro must be written like this:

```c
#define AHB1_EN_R (*(volatile unsigned int *)(RCC_BASE + AHB1_EN_OFFSET))
```

The `*` is important because it dereferences the pointer, allowing the program to access the actual hardware register at that memory address.

In this case, the code was already correct:

```c
AHB1_EN_R |= GPIOAEN;
```

The issue was not caused by the current source code. STM32CubeIDE was likely still using an old build state from before the macro was corrected.

The fix was to clean and rebuild the project:

```text
Project → Clean
Project → Build Project
```

After cleaning and rebuilding, the project compiled and ran successfully.

### Lesson learned

If the code looks correct but STM32CubeIDE still shows an error that seems to refer to an older version of the code, perform a clean build. This deletes old build artifacts and forces the IDE to recompile everything from the current source files.

This is especially useful after changing macros, register definitions, or header files in bare-metal embedded projects.
